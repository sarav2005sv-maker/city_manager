#include <stdio.h>
#include <string.h>
#include "city_hub.h"

int main(int argc, char** argv){
    if ((strcmp(argv[1], "start_monitor") == 0) && argc == 2){
        start_monitor();
    }
    return 0;
}
