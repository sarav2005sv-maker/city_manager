#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define SIZE 1000

//killall monitor_reports

void start_monitor(){
    int pipe_fd[2];

    if (pipe(pipe_fd) < 0) {
        exit(-1);
    }

    int pid_hub_mon = fork();
    if (pid_hub_mon < 0){
        fprintf(stderr, "Error at fork - start_monitor!\n");
        exit(-1);
    }

    if (pid_hub_mon == 0){
        int monitor_pid = fork();
        if (monitor_pid < 0){
            fprintf(stderr, "Error at fork - hub_mon!\n");
            exit(-1);
        }

        if (monitor_pid == 0){
            if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                exit(-1);
            }
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            execlp("./monitor_reports", "monitor_reports", (char*)NULL);
            fprintf(stderr, "Error at execlp - monitor_reports!\n");
            exit(-1);
        }

        close(pipe_fd[1]);

        char buf[SIZE];
        int already_read = 0;

        while ((already_read = read(pipe_fd[0], buf, sizeof(buf) - 1)) > 0){
            buf[already_read] = '\0';
            printf("Received message: %s", buf);
            fflush(stdout);
        }

        printf("Monitor turned off.\n");
        close(pipe_fd[0]);
        exit(0);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    printf("Monitor process runs in the background.\n");
}
