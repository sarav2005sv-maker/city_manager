#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "city_hub.h"

int active_hub_mon_pid = 0;

void handle_hub_sigint(int sig) {
    if (active_hub_mon_pid > 0) {
        printf("\n[city_hub] Intercepted Ctrl+C. Shutting down background monitor...\n");
        kill(active_hub_mon_pid, SIGINT);
        waitpid(active_hub_mon_pid, NULL, 0);
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    if (argc < 2) {
        fprintf(stderr, "Improper usage of city_hub!\n");
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_hub_sigint;
    sigaction(SIGINT, &sa, NULL);

    if ((strcmp(argv[1], "start_monitor") == 0) && argc == 2){
        active_hub_mon_pid = start_monitor();
        while(1) {
            pause();
        }
    }
    else
        if ((strcmp(argv[1], "calculate_scores") == 0)){
            calculate_scores(argc, argv);
            return 0;
        }
    else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
