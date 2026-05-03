#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#define INSPECTOR 0
#define MANAGER 1
#define CHUNK 30

typedef struct{
    int id;
    char inspector[50];
    float lat, longit;
    char issue[20];
    int severity;
    time_t time;
    char description[100];
}report;

typedef struct{
  time_t time;
  char role[20];
  char name[20];
  char action[20];
}logged_district_recording;

void print_report(report rep){
    printf("ID: %d\n", rep.id);
    printf("Inspector: %s\n", rep.inspector);
    printf("Lat: %.2f\n", rep.lat);
    printf("Long: %.2f\n", rep.longit);
    printf("Issue: %s\n", rep.issue);
    printf("Severity: %d\n", rep.severity);
    printf("Time: %ld\n", rep.time);
    printf("Description: %s\n", rep.description);
}

int verify_role(char* role){
  if (strcmp(role,"inspector") == 0) return INSPECTOR;
  if (strcmp(role,"manager") == 0) return MANAGER;
  fprintf(stderr, "Rol invalid --- exit\n");
  exit(-1);
}

void check_error_file(int fd){
  if (fd == -1){
  fprintf(stderr, "Error handling file --- exit\n");
  exit(-1);
  }
}

int get_permissions(char* pathname){
  struct stat st;
  if (lstat(pathname, &st) == -1){
    fprintf(stderr, "Error getting file information (lstat)\n");
    exit(-1);
  }
  mode_t perms = st.st_mode & 0777;
  return perms;
}

void get_symbolic_permissions(int mode, char *res) {
    res[0] = (mode & S_IRUSR) ? 'r' : '-';
    res[1] = (mode & S_IWUSR) ? 'w' : '-';
    res[2] = (mode & S_IXUSR) ? 'x' : '-';

    res[3] = (mode & S_IRGRP) ? 'r' : '-';
    res[4] = (mode & S_IWGRP) ? 'w' : '-';
    res[5] = (mode & S_IXGRP) ? 'x' : '-';

    res[6] = (mode & S_IROTH) ? 'r' : '-';
    res[7] = (mode & S_IWOTH) ? 'w' : '-';
    res[8] = (mode & S_IXOTH) ? 'x' : '-';
    res[9] = '\0';
}

void write_to_log(int permissions, int role, char** argv, int fd3){
  if ((role == MANAGER && (permissions & S_IWUSR) != 0) || ((role == INSPECTOR) && (permissions & S_IWGRP))){
    logged_district_recording* rec = malloc(1 * sizeof(*rec));
    rec->time = time(NULL);
    strcpy(rec->role, argv[2]);
    strcpy(rec->name, argv[4]);
    strncpy(rec->action, argv[5] + 2, 19);
    //rec->action[3] = '\0';
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%ld %s %s %s\n",rec->time, rec->role, rec->name, rec->action);
    write(fd3,buf,len);
    free(rec);
  }
  else {
    fprintf(stderr, "Permission denied - Trying to write to logged_district as unauthorized role!\n");
  }
}

void add_command(int argc, char** argv, int role){
  if (mkdir(argv[6], 0750) != 0 && errno != EEXIST) {
        perror("Error creating directory");
        exit(EXIT_FAILURE);
    }
  int fd1,fd2,fd3;
  char s1[50];
  sprintf(s1,"%s/%s",argv[6],"reports.dat");
  fd1 = open(s1,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  check_error_file(fd1);
  chmod(s1,0664);

  char link_name[100];
  sprintf(link_name, "active-reports-%s",argv[6]);
  unlink(link_name);
  if (symlink(s1, link_name) != 0){
      fprintf(stderr,"Error creating symbolic link\n");
}

  char s2[50];
  sprintf(s2,"%s/%s",argv[6],"district.cfg");
  fd2 = open(s2,O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP);
  close(fd2);
  char s3[50];
  sprintf(s3,"%s/%s",argv[6],"logged_district");
  fd3 = open(s3,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  check_error_file(fd3);

  report* report = malloc(1 * sizeof(*report));
  if (report == NULL){
    fprintf(stderr, "Err alocare\n");
    exit(-1);
  }
  printf("ID:\n");
  scanf("%d",&report->id);
  strcpy(report->inspector,argv[4]);
  printf("Latitudine:\n");
  scanf("%f",&report->lat);
  printf("Longitudine:\n");
  scanf("%f",&report->longit);
  printf("Issue:\n");
  scanf("%19s",report->issue);
  printf("Severity:\n");
  scanf("%d",&report->severity);
  report->time = time(NULL);
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
  printf("Description:\n");
  fgets(report->description, 100, stdin);
  write(fd1,report,sizeof(*report));

  int permissions = get_permissions(s3);
  write_to_log(permissions, role, argv, fd3);

  close(fd1);
  close(fd3);
  free(report);
}


void update_threshhold(int argc, char** argv, int role){
  int fd,fd3;
  if (role == MANAGER) {
    char s[50];
    sprintf(s,"%s/%s",argv[6],"district.cfg");
    fd = open(s,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR|S_IRGRP);
    check_error_file(fd);
    int permissions = get_permissions(s);
    if ((permissions & 0777) != 0640) {
    fprintf(stderr, "district.cfg permissions incorrect\n");
    exit(1);
    }
    else {
    char buf[32];
    int th = atoi(argv[7]);
    int len = snprintf(buf, sizeof(buf), "%d\n", th);
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, len);
    char s3[50];
    sprintf(s3,"%s/%s",argv[6],"logged_district");
    fd3 = open(s3,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  check_error_file(fd3);
    write_to_log(permissions, role, argv, fd3);
    }
  }
  else {
    fprintf(stderr, "Permission denied - Trying to write to district.cfg as INSPECTOR\n");
  }
  close(fd);
}

void list_command(char** argv, int role){
    char s[50];
    sprintf(s,"%s/%s",argv[6],"reports.dat");
    int perms = get_permissions(s);
    if (((role == MANAGER) && (perms & S_IRUSR)) ||
        ((role == INSPECTOR) && (perms & S_IRGRP)))
    {
    int fd = open(s,O_RDONLY);
    check_error_file(fd);
    struct stat st;
    if (stat(s,&st) == 0){
       char permissions[20];
       get_symbolic_permissions(perms,permissions);
       printf("Reports.dat: %s %ld %ld\n",permissions,st.st_size,st.st_mtime);
    }
    report rep;
    while (read(fd,&rep,sizeof(rep)) == sizeof(rep)){
        printf("ID: %d\n", rep.id);
        printf("Inspector: %s\n", rep.inspector);
        printf("Lat: %.2f\n", rep.lat);
        printf("Long: %.2f\n", rep.longit);
        printf("Issue: %s\n", rep.issue);
        printf("Severity: %d\n", rep.severity);
        printf("Time: %ld\n", rep.time);
        printf("Description: %s\n", rep.description);
    }
   }
   else {
       fprintf(stderr,"Error - Role does not have permission to read file!\n");
       exit(-1);
}
    char s3[50];
    sprintf(s3,"%s/%s",argv[6],"logged_district");
    int fd3 = open(s3,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    check_error_file(fd3);
    if (role == MANAGER){
        write_to_log(perms, role, argv, fd3);
    }
}

void view_command(char** argv, int role){
    char s[50];
    sprintf(s,"%s/%s",argv[6],"reports.dat");
    int perms = get_permissions(s);
    if (((role == MANAGER) && (perms & S_IRUSR)) ||
        ((role == INSPECTOR) && (perms & S_IRGRP)))
    {
    int fd = open(s,O_RDONLY);
    check_error_file(fd);
    report rep;
    int found = 0;
    while (read(fd, &rep, sizeof(report)) > 0) {
        if (rep.id == atoi(argv[7])) {
        print_report(rep);
        found = 1;
        break;
      }
    }
    if (!found) printf("Report not found.\n");
    }
   else {
       fprintf(stderr,"Error - Role does not have permission to read file!\n");
       exit(-1);
}
    char s3[50];
    sprintf(s3,"%s/%s",argv[6],"logged_district");
    int fd3 = open(s3,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    check_error_file(fd3);
    if (role == MANAGER){
        write_to_log(perms, role, argv, fd3);
    }
}

void remove_report(char** argv, int role){
    char s[50];
    sprintf(s,"%s/%s",argv[6],"reports.dat");
    int perms = get_permissions(s);
    if ((role == MANAGER) && (perms & S_IRUSR) && (perms & S_IWUSR))
    {
        int fd = open(s,O_RDWR);
        check_error_file(fd);
        int current_report_index = 0;
        report rep;
        int found = 0;
    while (read(fd, &rep, sizeof(report)) > 0) {
        if (rep.id != atoi(argv[7])) {
            current_report_index++;
            continue;
        }
        else{
            found = 1;
            lseek(fd,current_report_index * sizeof(report),SEEK_SET);
            lseek(fd,(current_report_index+1) * sizeof(report),SEEK_SET);
            report buf;
            while (read(fd, &buf, sizeof(report)) > 0) {
                lseek(fd, current_report_index * sizeof(report), SEEK_SET);
                write(fd, &buf, sizeof(report));
                current_report_index++;
                lseek(fd, (current_report_index + 1) * sizeof(report), SEEK_SET);
            }
        }
        struct stat st;
        if (lstat(s, &st) == 0) {
            long int size_in_bytes = st.st_size;
            printf("File size: %ld bytes\n", size_in_bytes);
        }
     }
    if (!found){
        fprintf(stderr,"Report does not exist - cannot delete!\n");
        exit(-1);
    }
    ftruncate(fd, current_report_index * sizeof(report));
    ftruncate(fd, current_report_index * sizeof(report));
    char s3[50];
    sprintf(s3,"%s/%s",argv[6],"logged_district");
    int fd3 = open(s3,O_CREAT|O_RDWR|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    check_error_file(fd3);
    write_to_log(perms, role, argv, fd3);
    }
    else {
      fprintf(stderr,"Trying to remove report as unauthorized role!\n");
      exit(-1);
    }
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1;
    }
    return 0;
}

int match_condition(report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0 || strcmp(field, "time") == 0 || strcmp(field, "id") == 0) {
        long record_val;
        if (strcmp(field, "severity") == 0) record_val = r->severity;
        else if (strcmp(field, "id") == 0) record_val = r->id;
        else record_val = (long)r->time;

        long filter_val = atol(value);

        if (strcmp(op, "==") == 0) return record_val == filter_val;
        if (strcmp(op, "!=") == 0) return record_val != filter_val;
        if (strcmp(op, "<") == 0)  return record_val < filter_val;
        if (strcmp(op, "<=") == 0) return record_val <= filter_val;
        if (strcmp(op, ">") == 0)  return record_val > filter_val;
        if (strcmp(op, ">=") == 0) return record_val >= filter_val;
    }
    else if (strcmp(field, "issue") == 0 || strcmp(field, "inspector") == 0) {
        const char *record_str = (strcmp(field, "issue") == 0) ? r->issue : r->inspector;
        int cmp = strcmp(record_str, value);

        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
        // Lexicographical comparison for strings
        if (strcmp(op, "<") == 0)  return cmp < 0;
        if (strcmp(op, ">") == 0)  return cmp > 0;
    }
    return 0;
}

//trebuie puse ghilimele la epel - "severity:>:1"
void filter_command(int argc, char** argv, int role) {
    char path[100];
    sprintf(path, "%s/reports.dat", argv[6]);

    int perms = get_permissions(path);

    if (!((role == MANAGER && (perms & S_IRUSR)) || (role == INSPECTOR && (perms & S_IRGRP)))) {
        fprintf(stderr, "Permission denied\n");
        return;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    report rep;
    while (read(fd, &rep, sizeof(report)) == sizeof(report)) {
        int matches_all = 1;

        for (int i = 7; i < argc; i++) {
            char field[50], op[10], val[100];
            if (parse_condition(argv[i], field, op, val)) {
                if (!match_condition(&rep, field, op, val)) {
                    matches_all = 0;
                    break;
                }
            }
        }

        if (matches_all) {
            print_report(rep);
            printf("--------------------------\n");
        }
    }
    close(fd);
}

void remove_district(char** argv, int role) {
    if (role != MANAGER) {
        fprintf(stderr, "Trying to delete district as unauthorized role!\n");
        exit(-1);
    }

    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "active-reports-%s", argv[6]);
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Error at fork.\n");
        exit(-1);
    }

    if (pid == 0) {
        printf("Child process deleting directory: %s\n", argv[6]);
        execlp("rm", "rm", "-rf", argv[6], (char *)NULL);
        fprintf(stderr,"Failure at execlp.\n");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("District directory '%s' deleted successfully.\n", argv[6]);

            if (unlink(symlink_name) == 0) {
                printf("Symlink '%s' removed.\n", symlink_name);
            } else {
                fprintf(stderr,"Could not remove symlink.\n");
            }
        } else {
            fprintf(stderr, "Error: rm command failed to delete district.\n");
            exit(EXIT_FAILURE);
        }
    }
}





















