#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include"arcFuns.h"
#define BUFFER_SIZE 4096

void archiveFiles(const char* source_path, const char* destination_path) {
    FILE* source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        printf("Error opening the source file.\n");
        return;
    }

    FILE* destination_file = fopen(destination_path, "ab");
    if (destination_file == NULL) {
        printf("Error creating the destination file.\n");
        fclose(source_file);
        return;
    }

    fprintf(destination_file, "Source File: %s\n", source_path);
    char buffer[BUFSIZ];
    size_t num_read;

    while ((num_read = fread(buffer, sizeof(char), BUFSIZ, source_file)) > 0) {
        size_t num_written = fwrite(buffer, sizeof(char), num_read, destination_file);
        if (num_written != num_read) {
            printf("Error writing to the destination file.\n");
            break;
        }
    }

    fprintf(destination_file, "\n");
    fclose(source_file);
    fclose(destination_file);

    // Delete the source file
   // fclose(source_file);  // Close the source file before deleting
    if (remove(source_path) != 0) {
        printf("Error deleting the source file.\n");
    }
}

void searchFileNames(const char *destination_file_path) {
    FILE* destination_file = fopen(destination_file_path, "r");
    if (destination_file == NULL) {
        printf("Error opening the destination file.\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    char* line;
    int file_count = 0;
    char file_names[100][256];  // Assuming maximum 100 files and file names up to 255 characters

    while ((line = fgets(buffer, sizeof(buffer), destination_file)) != NULL) {
        if (strncmp(line, "Source File: ", 13) == 0) {
            char* file_name = line + 13; // Skip the "Source File: " prefix
            file_name[strcspn(file_name, "\n")] = '\0'; // Remove the trailing newline character
            strncpy(file_names[file_count], file_name, sizeof(file_names[file_count]));
            file_count++;
        }
    }

    fclose(destination_file);

    printf("Archived files:\n");
    for (int i = 0; i < file_count; i++) {
        printf("%s\n", file_names[i]);
    }
}

void extractFiles(const char *destination_file_path) {
    FILE* destination_file = fopen(destination_file_path, "r");
    if (destination_file == NULL) {
        printf("Error opening the destination file.\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    char* line;
    char current_file_name[256] = ""; // Stores the current file name being extracted
    FILE* extraction_file = NULL;     // Stores the file pointer of the current extraction file

    while ((line = fgets(buffer, sizeof(buffer), destination_file)) != NULL) {
        if (strncmp(line, "Source File: ", 13) == 0) {
            char* file_name = line + 13; // Skip the "Source File: " prefix
            file_name[strcspn(file_name, "\n")] = '\0'; // Remove the trailing newline character

            // If a file is being extracted, close it before starting a new extraction
            if (extraction_file != NULL) {
                fclose(extraction_file);
                extraction_file = NULL;
            }

            // Open a new extraction file for the current copied file
            char extraction_file_path[256];
            snprintf(extraction_file_path, sizeof(extraction_file_path), "%s", file_name);

            extraction_file = fopen(extraction_file_path, "wb");
            if (extraction_file == NULL) {
                printf("Error creating the extraction file: %s\n", extraction_file_path);
                fclose(destination_file);
                return;
            }

            strncpy(current_file_name, file_name, sizeof(current_file_name));

            printf("Extracting file: %s\n", file_name);
        }
        else if (extraction_file != NULL) {
            fputs(line, extraction_file);
        }
    }

    // Close the last extraction file if one was being extracted
    if (extraction_file != NULL) {
        fclose(extraction_file);
        extraction_file = NULL;
    }

    fclose(destination_file);
}
