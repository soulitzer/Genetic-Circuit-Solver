#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

int* read_file(char* filename, int base, int length) {
    printf("Opening file: '%s' in base %d. Expect %d lines.\n", filename, base, length);
    FILE *fs;

    char ch, buffer[32];

    int i = 0, j = 0;
    int *data = malloc(length * sizeof(int));

    // Openning the file with file handler as fs
    fs = fopen(filename, "r");

    // Read the file unless the file encounters an EOF
    while(1){
        ch = fgetc(fs);

        if(ch == EOF){
            break;
        } else if(ch == '\n') {
            data[j] = strtoul(buffer, NULL, base);
            j++;

            memset(buffer, 0, sizeof(char)*32);
            i = 0;

            continue;
        } else {
            buffer[i] = ch;
            i++;
        }
    }

    printf("Done.");
    return data;
}
