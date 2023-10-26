#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"

// Extracts the filename from the full path
// e.g. path="./root_directories/root1/sub_1/WorldStage_2.txt"
// return value = "WorldStage_2.txt"
char* extract_filename(char* path) {
    char* filename = strrchr(path, '/');
    if(filename == NULL)
        return path;
    return filename + 1;
}

// Extracts the directory name after "root_directories"
// e.g. path="./root_directories/original_root1/sub_1/WorldStage_2.txt"
// return value = "root1/"
char* extract_root_directory(const char* path) {
    char* root_dir = strstr(path, "/root_directories/");
    if (root_dir == NULL){
        return NULL; // "root_directories" not found in path
    }

    char* start_of_root = root_dir + strlen("/root_directories/");
    char* end_of_root = strchr(start_of_root, '/');

    if (end_of_root == NULL){
        return strdup(start_of_root);  // In case the path ends with the root directory
    }
    int length = end_of_root - start_of_root + 1; // +1 to include the trailing slash
    char* result = (char*)malloc(length + 1);  // +1 for the null terminator
    strncpy(result, start_of_root, length);
    result[length] = '\0';  // Null-terminate the result string
    return result;
}

void remove_filepath_duplicate(char **dup_list, char **retain_list, int *size) {
    for (int i = 0; i < *size; i++) {
        for (int j = i + 1; j < *size; ) {
            if (strcmp(dup_list[i], dup_list[j]) == 0) {
                free(dup_list[j]);
                free(retain_list[j]);
                for (int k = j; k < *size - 1; k++) {
                    dup_list[k] = dup_list[k + 1];
                    retain_list[k] = retain_list[k+1];
                }
                // Set the last items to NULL after shifting
                dup_list[*size - 1] = NULL;
                retain_list[*size - 1] = NULL;
                (*size)--;
            } else {
                j++;
            }
        }
    }
}


void sanitize_dup_retain(char **dup_list, char **retain_list, int size) {
    for (int m = 0; m < size; m++) {
        for (int n = 0; n < size; n++) {
            if (strcmp(retain_list[m], dup_list[n]) == 0) {
                if (retain_list[m] != retain_list[n]) {
                    free(retain_list[m]);
                    retain_list[m] = dup_list[n];
                }
                break;
            }
        }
    }
}

int parse_hash(char * file_hashes, char** dup_list, char** retain_list) {
    char delim[] = "|";
    char array[32][128];
    int count = 0;

    memset(array, 0, sizeof(array));

    char *ptr = strtok(file_hashes, delim);
    while(ptr != NULL && count < 32) {
        strncpy(array[count], ptr, sizeof(array[count]) - 1);
        ptr = strtok(NULL, delim);
        count++;
    }

    int index = 0;

    for(int i = 1; i < count; i += 2) {
        int retain_index = i; // index of file to be retained

        // Look for other files with the same hash
        for (int j = i + 2; j < count; j += 2) {
            if (strcmp(array[i], array[j]) == 0) {
                
                if (retain_index == i) { // if first match, add i-1 to retain_list
                    retain_list[index] = malloc(sizeof(char) * 128);
                    if (!retain_list[index]) {
                        fprintf(stderr, "Memory allocation failed!\n");
                        exit(1);
                    }
                    strcpy(retain_list[index], array[retain_index - 1]);
                }
                
                // add j-1 to dup_list
                dup_list[index] = malloc(sizeof(char) * 128);
                if (!dup_list[index]) {
                    fprintf(stderr, "Memory allocation failed!\n");
                    exit(1);
                }
                strcpy(dup_list[index], array[j - 1]);
                
                index++;

                // set the hash to empty string to avoid duplication
                array[j][0] = '\0';
            }
        }

        // if no match found, retain_index will still be i, so just move on
    }

    // remove duplicate filepath in dup_list get from file_hashes
    remove_filepath_duplicate(dup_list,retain_list,&index);
    // files in retain_list might also appear in dup_list,
    // since one file may have more than 2 duplicates
    sanitize_dup_retain(dup_list,retain_list,index);

    return index;
}
