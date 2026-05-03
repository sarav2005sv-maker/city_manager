#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "commands.h"

int main(int argc, char** argv){
  int role = verify_role(argv[2]);

  if (((strcmp(argv[5],"--add")) == 0) && (argc == 7))
  add_command(argc, argv, role);
  if ((strcmp(argv[5],"--list")) == 0 && (argc == 7))
  list_command(argv, role);
  if ((strcmp(argv[5],"--view")) == 0 && (argc == 8))
  view_command(argv, role);
  if ((strcmp(argv[5],"--remove_report")) == 0 && (argc == 8))
  remove_report(argv, role);
  if ((strcmp(argv[5],"--update_threshhold")) == 0 && (argc == 8)) update_threshhold(argc, argv, role);
  if ((strcmp(argv[5],"--filter")) == 0 && (argc >= 8))
  filter_command(argc, argv, role);
  if ((strcmp(argv[5],"--remove_district")) == 0 && (argc == 7))
  remove_district(argv, role);
  return 0;
}
