#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#define SIZE 100
#define PID_FILE ".monitor_pid"

volatile int keep_running = 1; //se poate schimba in urma semnalelor

void handle_sigint(int sig) {
    keep_running = 0;
}

void handle_sigusr1(int sig) {
    const char *msg = "EVENT - A new report has been added.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    int fd_check = open(PID_FILE,O_RDONLY);
    if (fd_check != -1){
        char existing_pid[SIZE];
        int size = read(fd_check,existing_pid,sizeof(existing_pid) - 1);
        close(fd_check);
        if(size > 0){
            existing_pid[size] = '\0';
            pid_t old_pid = (pid_t)atoi(existing_pid);
        if (kill(old_pid, 0) == 0) //verificam daca putem trimite semnale
        {
            if (old_pid != getpid()){
            printf("ERROR - Monitor already running - PID - %d\n", old_pid);
            fflush(stdout);
            exit(-1);
            }
        } else {
            //verificam daca a dat crash si nu a apucat sa stearga pid-file-ul
            if (errno == ESRCH) {
                printf("Found stale PID file (PID %d is dead). Cleaning up...\n", old_pid);
                unlink(PID_FILE);
            }
        }
      }
    }

    int fd = open(PID_FILE, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd != -1) {
        char pid_string[SIZE];
        int pid = getpid();
        int len = sprintf(pid_string, "%d", pid);
        if (write(fd, pid_string, len) == -1) {
            perror("Failed to write PID to file");
        }
        close(fd);
    } else {
        perror("Failed to open .monitor_pid");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_int, sa_usr1;

    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_usr1, 0, sizeof(sa_usr1));

    sa_int.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa_int, NULL);

    sa_usr1.sa_handler = handle_sigusr1;
    sa_usr1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    printf("INFO - Monitor started (PID: %d). Waiting for reports...\n", getpid());
    //printf("Press Ctrl+C to stop.\n");
    fflush(stdout);
    while (keep_running) {
        pause();
    }

    printf("INFO - SIGINT received. Shutting down.\n");
    fflush(stdout);
    unlink(PID_FILE); // Delete the .monitor_pid file

    return 0;
}
