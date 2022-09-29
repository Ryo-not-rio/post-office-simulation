#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./headers/main.h"

pair* readParameters(char fileName[], char *allowedParams[], int numParams) {    
    pair *parameters;
    if (!(parameters = (pair *)malloc(sizeof(pair)*numParams))) {
        printf("Could not allocate memory for parameters array\n");
        exit(1);
    }
    
    FILE *fptr;
    if (!(fptr = fopen(fileName, "r"))) {
        printf("Error opening params file");
        exit(1);
    }

    /* Reading each parameter from parameters file */
    char line[128];
    int index = 0;
    char searchString[50];
    while (fgets(line, 128, fptr)) {
        strcpy(searchString, allowedParams[index]);
        if (line[0] != '#' && strstr(line, searchString)) {
            float item1;
            float item2=0; /* Prevent unassigned value in parameters array */
            sscanf(line, " %*s %f ~ %f", &item1, &item2);   
            parameters[index].item1 = item1;
            parameters[index].item2 = item2;
            index++;
        }
    }

    if (index != numParams) {
        printf("One or more parameters not found in file. Exiting...");
        exit(1);
    }

    fclose(fptr);
    return parameters;
}