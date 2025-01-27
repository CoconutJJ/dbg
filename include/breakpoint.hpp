#pragma once
#include <unistd.h>

#include <cstdint>
class Breakpoint {
    public:
        Breakpoint (pid_t pid, std::intptr_t addr);

        void enable ();
        void disable ();

        bool is_enabled ();
        std::intptr_t get_address ();

    private:
        pid_t m_pid;
        std::intptr_t m_addr;
        bool m_enabled;
        uint32_t m_saved_data;
};