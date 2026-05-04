#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define SIZE 100

int main(){
    int fd = open(".monitor_pid", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd != -1){
        char pid_string[SIZE];
        int pid = getpid();
        int len = sprintf(pid_string, "%d", pid);
        write(fd, pid_string, len);
        close(fd);
    }
    struct sigaction sa_int, sa_usr1;
    memset(&sa_int, 0x00, sizeof(struct sigaction));
    memset(&sa_usr1, 0x00, sizeof(struct sigaction));
    sa_int.sa_handler = SIGINT;
    sa_usr1.sa_handler = SIGUSR1;

    return 0;
}
