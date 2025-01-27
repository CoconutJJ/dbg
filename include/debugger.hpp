#pragma once
#include "breakpoint.hpp"
#include <csignal>
#include <cstdint>
#include <string>
#include "dwarf++.hh"
#include "elf++.hh"
#include <sys/ptrace.h>

#define REG_BP 29
#define REG_LR 30

class Debugger {

    public:
        Debugger(std::string program_name);
        dwarf::die get_function_from_pc(uint64_t pc);
        dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
        int wait_for_signal();
        bool launch_tracee();
        void continue_execution();
        void run();
        void initialize_load_address();
        uint64_t offset_load_address(uint64_t address);

        uint64_t read_register(int reg_no);
        void write_register(int reg_no, uint64_t value);
        uint64_t get_pc();
        void write_pc(uint64_t address);


        uint64_t read_memory(uint64_t address);
        void write_memory (uint64_t address, uint64_t value);

        void set_breakpoint(uint64_t address);
        void unset_breakpoint(uint64_t address);
        void remove_breakpoint(uint64_t address);
        bool has_breakpoint(uint64_t address);


        void step_over_breakpoint();
        void single_step_instruction();
        void single_step_instruction_with_breakpoint_check();
        void step_out();

    private:
        pid_t m_pid;
        dwarf::dwarf m_dwarf;
        std::string m_program_name;
        elf::elf m_elf;
        uint64_t m_load_address;
        std::map<uint64_t, Breakpoint *> m_breakpoints;
};