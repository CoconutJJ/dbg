#include "breakpoint.hpp"
#include <cstdint>
#include <stdint.h>
#include <sys/ptrace.h>

Breakpoint::Breakpoint (pid_t pid, std::intptr_t addr) : m_pid (pid), m_addr (addr), m_enabled (false), m_saved_data{}
{
}

bool Breakpoint::is_enabled ()
{
        return this->m_enabled;
}

std::intptr_t Breakpoint::get_address ()
{
        return this->m_addr;
}

void Breakpoint::enable ()
{
        auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_addr, nullptr);

        m_saved_data = static_cast<uint32_t> (data & 0xffffffff);

        data = (data & ~0xffffffff) | 0x007d20d4;

        ptrace (PTRACE_POKEDATA, m_pid, m_addr, data);

        m_enabled = true;
}

void Breakpoint::disable ()
{
        auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_addr, nullptr);

        data = (data & ~0xffffffff) | m_saved_data;

        ptrace (PTRACE_POKEDATA, m_pid, m_addr, data);
}
