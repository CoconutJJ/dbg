#include <asm/ptrace.h>
#include <csignal>
#include <cstdlib>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

int main (int argc, char **argv)
{
        int optindex = 0;
        struct option longopts[] = {
                { .name = "prog", .has_arg = 1, .val = 'p' },
                { 0 }
        };

        char *debug_program_path = NULL;

        int c;
        while ((c = getopt_long (argc, argv, "", longopts, &optindex)) != -1) {
                switch (c) {
                case 'p': debug_program_path = optarg; break;
                }
        }

        struct user_regs_struct regs;
}
