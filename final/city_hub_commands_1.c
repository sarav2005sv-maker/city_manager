#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#define SIZE 1000

int start_monitor(){
    int pipe_fd[2];

    if (pipe(pipe_fd) < 0) {
        perror("Pipe creation failed");
        exit(-1);
    }

    int pid_hub_mon = fork();
    if (pid_hub_mon < 0){
        fprintf(stderr, "Error at fork - start_monitor!\n");
        exit(-1);
    }

    if (pid_hub_mon == 0){
        signal(SIGINT, SIG_IGN);

        int monitor_pid = fork();
        if (monitor_pid < 0){
            fprintf(stderr, "Error at fork - hub_mon!\n");
            exit(-1);
        }

        if (monitor_pid == 0){
            close(pipe_fd[0]);
            if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                exit(-1);
            }
            close(pipe_fd[1]);

            execlp("./monitor_reports_1", "monitor_reports_1", (char*)NULL);
            fprintf(stderr, "Error at execlp - monitor_reports!\n");
            exit(-1);
        }

        close(pipe_fd[1]);

        char buf[SIZE];
        int already_read = 0;

        while ((already_read = read(pipe_fd[0], buf, sizeof(buf) - 1)) > 0){
            buf[already_read] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }

        close(pipe_fd[0]);

        int status;
        waitpid(monitor_pid, &status, 0);

        printf("Monitor turned off.\n");
        exit(0);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    return pid_hub_mon;
}

void calculate_scores(int argc, char** argv) {
    printf("\nCOMBINED URBAN REGION WORKLOAD\n\n");

    for (int i = 2; i < argc; i++) {
        char* district_folder = argv[i];
        int pipe_fd[2];

        if (pipe(pipe_fd) < 0) {
            perror("Pipe generation failure");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork generation failure");
            continue;
        }

        if (pid == 0) {
            close(pipe_fd[0]);
            if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
                perror("dup2 failed");
                exit(-1);
            }
            close(pipe_fd[1]);
            execlp("./scorer", "scorer", district_folder, (char*)NULL);
            perror("execlp execution failed");
            exit(-1);
        }
        else {
            close(pipe_fd[1]);

            char buffer[512];
            int bytes_read;
            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            }

            close(pipe_fd[0]);
            waitpid(pid, NULL, 0);
        }
    }
    printf("\n");
}









