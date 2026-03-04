#include "file_explorer.h"
#include "client.h"
#include "helpers.h"

int file_explorer(char* path, char* application_str, char* description_str, char* file) {

    char curr_path[PATH_MAX];
    strncpy(curr_path, path, sizeof(curr_path));
    // Just in case
    curr_path[PATH_MAX-1] = '\0';

    struct dirent *dir_entry[2048];
    int n_entries = 0;
    int exit = 0;
    char option = 0;
    int selected_index = -1;

    while (!exit) {

        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/           *** File Explorer ***          /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Application: '%s'\n", application_str);
        printf_s(" Detail: '%s'\n", description_str);
        printf_s("\n");
        printf_s(" Current path: '%s'\n", curr_path);
        printf_s(" Please select an option\n");
        printf_s("-------------------------------------------\n");
        printf_s("\n");
        printf_s("[z] Exit\n\n");

        n_entries = dir_navigation(curr_path, dir_entry, 0);
        int entries = n_entries > 35 ? 35 : n_entries;
        for(int i=0 ; i<entries ; i++) {
            if (i<=9)
                printf_s("[%d] ", i);
            else
                printf_s("[%c] ", 'a'+(char)i-10);
            printf_s("%s %s\n", dir_entry[i]->d_type==DT_DIR?"(folder)":"(file)  ", dir_entry[i]->d_name);
        }
    
        printf_s("\nSelection: ");

        option = read_char();

        // If we receive LF or CR we do nothing
        if(option==0x0A || option==0x0D)
            continue;

        if (option == 'z')
            exit = 1;
        if (option >= '0' && option <= '9') {
            selected_index = option - (int) '0';
        }
        if (option >= 'a' && option <= 'y') {
            selected_index = option - (int) 'a' + 10;
        }
        if (!exit && selected_index != -1) {
            // If it's a file we finished
            if (dir_entry[selected_index]->d_type == DT_REG) {
                strcpy(file, curr_path);
                append_and_clean_path(file, dir_entry[selected_index]->d_name);
                return 0;
            }
            else {
                append_and_clean_path(curr_path, dir_entry[selected_index]->d_name);
            }
        }
        selected_index = -1;
    }

    return 1;
}

int folder_explorer(char* path, char* application_str, char* description_str, char* folder) {

    char curr_path[PATH_MAX];
    strncpy(curr_path, path, sizeof(curr_path));
    // Just in case
    curr_path[PATH_MAX-1] = '\0';

    struct dirent *dir_entry[2048];
    int n_entries = 0;
    int exit = 0;
    char option = 0;
    int selected_index = -1;

    while (!exit) {

        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/           *** Folder Explorer ***          /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Application: '%s'\n", application_str);
        printf_s(" %s\n", description_str);
        printf_s("\n");
        printf_s(" Current path: '%s'\n", curr_path);
        printf_s(" Please select an option\n");
        printf_s("-------------------------------------------\n");
        printf_s("\n");
        printf_s("[z] Exit\n\n");
        printf_s("[0] Use this folder\n\n");

        n_entries = dir_navigation(curr_path, dir_entry, 1);
        int entries = n_entries > 35 ? 35 : n_entries;
        for(int i=1 ; i<=entries ; i++) {
            if (i<=9)
                printf_s("[%d] ", i);
            else
                printf_s("[%c] ", 'a'+(char)i-10);
            printf_s("%s\n", dir_entry[i-1]->d_name);
        }
    
        printf_s("\nSelection: ");

        option = read_char();
        printf_s("Selected: %c\n", option);

        // If we receive LF or CR we do nothing
        if(option==0x0A || option==0x0D)
            continue;

        if (option == 'z')
            exit = 1;
        if (option == '0') {
            selected_index = 99;
        }
        if (option >= '1' && option <= '9') {
            selected_index = option - (int) '1';
        }
        if (option >= 'a' && option <= 'y') {
            selected_index = option - (int) 'a' + 10;
        }
        if (!exit && selected_index != -1) {
            // If it's a file we finished
            if (selected_index == 99) {
                strcpy(folder, curr_path);
                return 0;
            }
            else {
                append_and_clean_path(curr_path, dir_entry[selected_index]->d_name);
            }
        }
        selected_index = -1;
    }
    return 1;
}

void append_and_clean_path(char* path, char* node) {
    char clean_path[PATH_MAX];
    strlcat(path, "/", PATH_MAX);
    strlcat(path, node, PATH_MAX);
    realpath(path, clean_path);             // Clean Path
    strncpy(path, clean_path, PATH_MAX);    // Copy to path
}

int dir_navigation(char* current_path, struct dirent **dir_entry, int hide_files) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(current_path);
    if (dir == NULL) {
        printf("Error listing dir '%s'", current_path);
        return -1;
    }

    int index=0;
    // Read first
    entry = readdir(dir);
    while (entry != NULL) {
        // Exclude "current dir"
        if(strcmp(entry->d_name, ".") != 0 && (entry->d_type == DT_DIR || (entry->d_type == DT_REG && hide_files == 0))) {
            dir_entry[index] = entry;
            index++;
        }
        // Read next
        entry = readdir(dir);        
    }

    // Order by Folder/File and Name
    for (int i=0 ; i<(index-1) ; i++) {
        for (int j=i+1 ; j<index ; j++) {
            int exchange=0;
            // Folder / File
            if (dir_entry[i]->d_type > dir_entry[j]->d_type) {
                exchange = 1;
            } 
            // If same type -> By Name
            else if (dir_entry[i]->d_type == dir_entry[j]->d_type) {
                if (strcmp(dir_entry[i]->d_name, dir_entry[j]->d_name) > 0) {
                    exchange = 1;
                }
            }
            if (exchange) {
                entry = dir_entry[j];
                dir_entry[j] = dir_entry[i];
                dir_entry[i] = entry;
            }
        }
    }

    closedir(dir);
    return index;
}