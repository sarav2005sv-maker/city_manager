#include <stdio.h>
#include <sys/stat.h>
#define inspector 0
#define manager 1

typedef struct{
    int id;
    float latitude, longitude;
    char issue_category[20];
    int severity;
    time_t timestamp;
    char description[100];
}report;

int main(int argc, char** argv){
    int role;
    if (strcmp(argv[2],"inspector") == 0) role = inspector;
    if (strcmp(argv[2],"manager") == 0) role = manager;
    switch(argv[3]){
        case("--add"):
        {
           mkdir(argv[4],S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP);
           char s[50];
           sprintf(s,"%s/%s",argv[4],"reports.dat");
           open(s,O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
        }
    }
    return 0;
}
