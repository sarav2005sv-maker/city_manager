#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define SIZE 1000

void start_monitor(){
    int pipe_fd[2];
    pipe(pipe_fd);
    int pid_hub_mon;
    if ((pid_hub_mon = fork()) < 0){
        fprintf(stderr, "Error at fork - start_monitor!\n");
        exit(-1);
    }
    if (pid_hub_mon == 0){
        int monitor_pid;
        close(pipe_fd[1]); //doar citim, inchidem capatul de write
        if ((monitor_pid = fork()) < 0){
            fprintf(stderr, "Error at fork - hub_mon!\n");
            exit(-1);
        }
        if (monitor_pid == 0){
            dup2(pipe_fd[1],1); //capatul de write devine stdout
            close(pipe_fd[0]); //doar scriem, inchidem capatul de read
            close(pipe_fd[1]);
            execlp("./monitor_reports","monitor_reports",(char*)NULL);
            fprintf(stderr,"Error at execlp - monitor_reports!\n");
            exit(-1);
        }
        close(pipe_fd[1]);
        char buf[SIZE];
        int already_read = 0;
        while ((already_read = read(pipe_fd[0],buf,sizeof(buf)-1)) > 0){
            buf[already_read] = '\0';
            printf("Received message: %s",buf);
        }
        //cand nu citim nmk, aka ENTER,programul se opreste
        printf("Monitor turned off.\n");
        close(pipe_fd[0]);
        exit(-1);
    }
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    printf("Monitor process runs in the background.\n");
}
