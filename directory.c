#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "directory.h"//if error occur erase this line only!!
#define MAX_LINE_LENGTHDIR 512


void highlight_word(const char *line, const char *word) {
    const char *match_start = strstr(line, word);
    while (match_start != NULL) {
        int word_length = strlen(word);
        int match_index = match_start - line;
        printf("%.*s", match_index, line);         // Print the portion before the match
        printf("\033[1;31m%.*s\033[0m", word_length, match_start);    // Print the highlighted word

        line = match_start + word_length;
        match_start = strstr(line, word);
    }
    printf("%s", line);      // Print the remaining portion of the line
}




void search_word_in_file(const char *file_path, const char *word) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", file_path);
        return;
    }

    printf("File: %s\n", file_path);           // Print the file name

    char line[MAX_LINE_LENGTHDIR];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, word) != NULL) {
            highlight_word(line, word);
        }
    }

    fclose(file);

    printf("\n");        // Add a new line after each file's contents
}


void search_word_in_directory(const char *directory_path, const char *word) {
    DIR *dir;
    struct dirent *entry;	

    dir = opendir(directory_path);
    if (dir == NULL) {
        printf("Error opening directory: %s\n", directory_path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char subdirectory_path[1024];
            snprintf(subdirectory_path,sizeof(subdirectory_path), "%s/%s", directory_path, entry->d_name);
            subdirectory_path[sizeof(subdirectory_path) - 1] = '\0';
            search_word_in_directory(subdirectory_path, word);
        } else if (entry->d_type == DT_REG) {
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);

            search_word_in_file(file_path, word);
        }
    }

    closedir(dir);
}
