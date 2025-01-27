#include "debugger.hpp"
#include "breakpoint.hpp"
#include "data.hh"
#include "dwarf++.hh"
#include "elf++.hh"
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

Debugger::Debugger (std::string program_name) : m_program_name (program_name)
{
        int program_file_fd = open (program_name.c_str (), O_RDONLY);

        if (program_file_fd < 0) {
                perror ("open");
                exit (EXIT_FAILURE);
        }

        m_elf = elf::elf (elf::create_mmap_loader (program_file_fd));
        m_dwarf = dwarf::dwarf (dwarf::elf::create_loader (m_elf));
}

dwarf::die Debugger::get_function_from_pc (uint64_t pc)
{
        for (auto &cu : m_dwarf.compilation_units ()) {
                if (!dwarf::die_pc_range (cu.root ()).contains (pc))
                        continue;
                for (const dwarf::die &die : cu.root ()) {
                        if (die.tag != dwarf::DW_TAG::subprogram)
                                continue;

                        if (!dwarf::die_pc_range (die).contains (pc))
                                continue;

                        return die;
                }
        }

        throw std::out_of_range ("Cannot find function");
}

dwarf::line_table::iterator Debugger::get_line_entry_from_pc (uint64_t pc)
{
        for (auto &cu : m_dwarf.compilation_units ()) {
                if (!dwarf::die_pc_range (cu.root ()).contains (pc))
                        continue;

                auto &lt = cu.get_line_table ();

                auto it = lt.find_address (pc);

                if (it == lt.end ())
                        throw std::out_of_range ("Cannot find line entry!");
                
                
                return it;
        }
}

void Debugger::set_breakpoint (uint64_t address)
{
        Breakpoint *bp = nullptr;
        if (this->has_breakpoint (address)) {
                bp = this->m_breakpoints[address];

                if (bp->is_enabled ())
                        return;

        } else {
                bp = new Breakpoint (this->m_pid, address);
        }

        bp->enable ();

        this->m_breakpoints[address] = bp;
}

void Debugger::unset_breakpoint (uint64_t address)
{
        Breakpoint *bp = nullptr;
        if (!this->has_breakpoint (address))
                return;

        bp = this->m_breakpoints[address];

        if (!bp->is_enabled ())
                return;

        bp->disable ();
}

void Debugger::remove_breakpoint (uint64_t address)
{
        if (!this->has_breakpoint (address))
                return;

        this->unset_breakpoint (address);

        this->m_breakpoints.erase (address);
}

bool Debugger::has_breakpoint (uint64_t address)
{
        return this->m_breakpoints.count (address) > 0;
}

void Debugger::step_over_breakpoint ()
{
        uint64_t pc = this->get_pc ();

        uint64_t bp_loc = pc - 1;

        if (!this->m_breakpoints.count (bp_loc))
                return;

        this->write_pc (bp_loc);

        this->unset_breakpoint (bp_loc);

        ptrace (PTRACE_SINGLESTEP, this->m_pid, nullptr, nullptr);

        this->wait_for_signal ();

        this->set_breakpoint (bp_loc);
}

void Debugger::single_step_instruction ()
{
        ptrace (PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
        this->wait_for_signal ();
}

void Debugger::single_step_instruction_with_breakpoint_check ()
{
        if (m_breakpoints.count (this->get_pc ())) {
                this->step_over_breakpoint ();
        } else {
                this->single_step_instruction ();
        }
}

void Debugger::step_out ()
{
        uint64_t ret_addr = this->read_register (REG_LR);

        bool should_remove_breakpoint = true;

        if (this->has_breakpoint (ret_addr))
                should_remove_breakpoint = false;

        this->set_breakpoint (ret_addr);

        this->continue_execution ();

        if (should_remove_breakpoint) 
                this->remove_breakpoint (ret_addr);

}

uint64_t Debugger::read_memory (uint64_t address)
{
        return ptrace (PTRACE_PEEKDATA, m_pid, address, nullptr);
}

void Debugger::write_memory (uint64_t address, uint64_t value)
{
        ptrace (PTRACE_POKEDATA, m_pid, address, value);
}

uint64_t Debugger::get_pc ()
{
        return this->read_register (-1);
}

void Debugger::write_pc (uint64_t address)
{
        this->write_register (-1, address);
}

uint64_t Debugger::read_register (int reg_no)
{
        struct user_regs_struct regs;

        ptrace (PTRACE_GETREGSET, this->m_pid, nullptr, &regs);

        if (reg_no == -1)
                return regs.pc;

        return regs.regs[reg_no];
}

void Debugger::write_register (int reg_no, uint64_t value)
{
        struct user_regs_struct regs;

        ptrace (PTRACE_GETREGSET, this->m_pid, nullptr, &regs);

        if (reg_no == -1) {
                regs.pc = value;
        } else {
                regs.regs[reg_no] = value;
        }

        ptrace (PTRACE_SETREGSET, this->m_pid, nullptr, &regs);
}

int Debugger::wait_for_signal ()
{
        int status;

        waitpid (this->m_pid, &status, 0);

        return status;
}

bool Debugger::launch_tracee ()
{
        pid_t pid = fork ();

        if (pid < 0) {
                perror ("fork");
                return false;
        }
        if (pid > 0) {
                m_pid = pid;
                return true;
        }
        ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
        personality (ADDR_NO_RANDOMIZE);
        execl (this->m_program_name.c_str (), this->m_program_name.c_str (), nullptr);
        perror ("exec");
        raise (SIGSTOP);
}

void Debugger::continue_execution ()
{
        ptrace (PTRACE_CONT, this->m_pid, nullptr, nullptr);
        this->wait_for_signal ();
}

void Debugger::initialize_load_address ()
{
        if (m_elf.get_hdr ().type != elf::et::dyn)
                return;

        std::ifstream map ("/proc/" + std::to_string (this->m_pid) + "/maps");

        std::string addr;

        std::getline (map, addr, '-');

        this->m_load_address = std::stoi (addr, 0, 16);
}

uint64_t Debugger::offset_load_address (uint64_t address)
{
        return address - this->m_load_address;
}

void Debugger::run ()
{
        if (!this->launch_tracee ()) {
                exit (EXIT_FAILURE);
        }

        while (1) {
                int wait_status = this->wait_for_signal ();

                if (WIFSTOPPED (wait_status)) {
                        fprintf (stderr, "Process received signal %d\n", WSTOPSIG (wait_status));
                }

                // if (WIFEXITED(wait_status)) {
                //   int status = WEXITSTATUS(wait_status);
                //   fprintf(stderr, "Process exited with status code %d\n", status);
                //   break;
                // } else if (WIFSIGNALED(wait_status)) {
                //   fprintf(stderr, "Process received signal %d\n", WTERMSIG(wait_status));
                // }

                getchar ();
                ptrace (PTRACE_CONT, this->m_pid, nullptr, nullptr);
        }
}
