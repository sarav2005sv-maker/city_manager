#include <stdio.h>

typedef struct{
    int id;
    char inspector[50];
    float lat, longit;
    char issue[20];
    int severity;
    time_t time;
    char description[100];
}report;

void print_report(report r);
int verify_role(char* role);
void check_error_file(int fd);
int get_permissions(char* pathname);
void write_to_log(int permissions, int role, char** argv, int fd3);
void add_command(int argc, char** argv, int role);
void list_command(char** argv, int role);
void view_command(char** argv, int role);
void update_threshhold(int argc, char** argv, int role);
void remove_report(char** argv, int role);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(report *r, const char *field, const char *op, const char *value);
void filter_command(int argc, char** argv, int role);
