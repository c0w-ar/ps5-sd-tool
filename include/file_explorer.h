#ifndef __FILE_EXPLORER_H__
#define __FILE_EXPLORER_H__

#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

int file_explorer(char* path, char* application_str, char* description_str, char* file);
int folder_explorer(char* path, char* application_str, char* description_str, char* folder);
int dir_navigation(char* current_path, struct dirent **dir_entry, int hide_files);
void append_and_clean_path(char* path, char* folder);

#endif