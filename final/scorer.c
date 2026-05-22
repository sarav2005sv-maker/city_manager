#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INSPECTORS 100

typedef struct {
    int id;
    char inspector[50];
    float lat, longit;
    char issue[20];
    int severity;
    time_t time;
    char description[100];
} report;

typedef struct {
    char name[50];
    int total_score;
} InspectorScore;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Scorer Error: Missing district folder argument\n");
        return 1;
    }

    char* district_folder = argv[1];
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district_folder);

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        printf("District [%s]: No reports.dat file found.\n\n", district_folder);
        return 0;
    }

    InspectorScore scores[MAX_INSPECTORS];
    int inspector_count = 0;
    report rep;
    while (fread(&rep, sizeof(report), 1, file) == 1) {
        if (rep.id == 0 || strlen(rep.inspector) == 0) {
            continue;
        }

        int found = 0;
        for (int i = 0; i < inspector_count; i++) {
            if (strcmp(scores[i].name, rep.inspector) == 0) {
                scores[i].total_score += rep.severity;
                found = 1;
                break;
            }
        }
        if (!found && inspector_count < MAX_INSPECTORS) {
            strncpy(scores[inspector_count].name, rep.inspector, 50);
            scores[inspector_count].total_score = rep.severity;
            inspector_count++;
        }
    }
    fclose(file);
    printf("--- Workload Summary for District: %s ---\n", district_folder);
    if (inspector_count == 0) {
        printf("No active data recorded.\n");
    } else {
        for (int i = 0; i < inspector_count; i++) {
            printf("Inspector: %s | Workload Score: %d\n", scores[i].name, scores[i].total_score);
        }
    }
    printf("\n");

    return 0;
}
