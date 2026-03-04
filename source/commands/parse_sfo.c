#include "parse_sfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sfo.h"
#include "client.h"
#include "helpers.h"
#include "client.h"
#include "db.h"
#include "show_users.h"
#include "file_explorer.h"
#include "sd.h"

extern user user_list[16]; // Global from show_users.c

void parse_sfo(char* file, savedata* sd) {
    printf_s("\x1b[2J\x1b[H");    // Clear screen
    printf_s(" --------------------------------------------\n");
    printf_s("/             *** SFO Parser ***            /\n");
    printf_s("-------------------------------------------\n");
    printf_s(" Selected File: '%s'\n", file);
    printf_s("-------------------------------------------\n");

    int fd = open(file, O_RDONLY);

    if (fd == -1) {
        printf_s("File '%s' not found\n", file);
        //return 1;
    }
    printf_s("Successfully opened: '%s'\n", file);

    // Get total size
    struct stat st;
    int sfo_file_size=0;

    fstat(fd, &st);
    sfo_file_size = st.st_size;

    sfo_header *sfo_hdr;
    sfo_index_metadata *sfo_meta;
    void* sfo_keys_name;
    void* sfo_keys_data;
    sfo_key* sfo_keys;


    sfo_hdr = malloc(sizeof(sfo_header));
    read(fd, sfo_hdr, sizeof(sfo_header));

    int size_meta = sizeof(sfo_index_metadata) * sfo_hdr->n_entries;
    sfo_meta = malloc(size_meta);
    read(fd, sfo_meta, size_meta);

    int size_keys_name = sfo_hdr->data_t_offset - sizeof(sfo_header) - size_meta;
    sfo_keys_name = malloc(size_keys_name);
    read(fd, sfo_keys_name, size_keys_name);

    int size_keys_data = sfo_file_size - sfo_hdr->data_t_offset;
    sfo_keys_data = malloc(size_keys_data);
    read(fd, sfo_keys_data, size_keys_data);

    sfo_keys = malloc(sizeof(sfo_key) * sfo_hdr->n_entries);


    for(int i=0; i<sfo_hdr->n_entries; i++) {
        sfo_key* key = &sfo_keys[i];
        key->name = (char*) sfo_keys_name + sfo_meta[i].key_offset;
        key->data_size = sfo_meta[i].param_length;
        key->data = (uint8_t*) sfo_keys_data + sfo_meta[i].data_offset;
    }

    printf_s("\n\n");
    printf_s("Index |         Name        |    Value\n");
    printf_s("---------------------------------------------------------------\n");
    for (int i=0; i<sfo_hdr->n_entries; i++) {
        
        printf_s("  %02d  | %-19s | ", i, sfo_keys[i].name);
        if (sfo_meta[i].param_format == 0x204) {
            replace_char((char*) sfo_keys[i].data, '\n', 'n');
            printf_s("'%s'", sfo_keys[i].data);
        }
        else {
            printf_s("0x");
            int trunc = sfo_keys[i].data_size > 16;
            int size = trunc ? 16 : sfo_keys[i].data_size;
            for (int j=0; j<size; j++)
                printf_s("%02X", sfo_keys[i].data[j]);
            if (trunc)
                printf_s(" (trunc) ");
        }
        printf_s("\n");
    }
    printf_s("\n\n");
    free(sfo_hdr);
    free(sfo_meta);
    free(sfo_keys_name);
    free(sfo_keys_data);
    free(sfo_keys);
    close(fd);
}

int parse_sfo_info(char* file, savedata* sd) {

    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        printf_s("File '%s' not found\n", file);
        return 1;
    }

    // Get total size
    struct stat st;
    int sfo_file_size=0;

    fstat(fd, &st);
    sfo_file_size = st.st_size;

    sfo_header *sfo_hdr;
    sfo_index_metadata *sfo_meta;
    void* sfo_keys_name;
    void* sfo_keys_data;
    sfo_key* sfo_keys;


    sfo_hdr = malloc(sizeof(sfo_header));
    read(fd, sfo_hdr, sizeof(sfo_header));

    int size_meta = sizeof(sfo_index_metadata) * sfo_hdr->n_entries;
    sfo_meta = malloc(size_meta);
    read(fd, sfo_meta, size_meta);

    int size_keys_name = sfo_hdr->data_t_offset - sizeof(sfo_header) - size_meta;
    sfo_keys_name = malloc(size_keys_name);
    read(fd, sfo_keys_name, size_keys_name);

    int size_keys_data = sfo_file_size - sfo_hdr->data_t_offset;
    sfo_keys_data = malloc(size_keys_data);
    read(fd, sfo_keys_data, size_keys_data);

    sfo_keys = malloc(sizeof(sfo_key) * sfo_hdr->n_entries);

    for(int i=0; i<sfo_hdr->n_entries; i++) {
        sfo_key* key = &sfo_keys[i];
        key->name = (char*) sfo_keys_name + sfo_meta[i].key_offset;
        key->data_size = sfo_meta[i].param_length;
        key->data = (uint8_t*) sfo_keys_data + sfo_meta[i].data_offset;
    }

    for (int i=0; i<sfo_hdr->n_entries; i++) {

        char* key = sfo_keys[i].name;
        void* data = sfo_keys[i].data;
        uint32_t d_size = sfo_keys[i].data_size;

        if(sd) {
            if(strcmp(key, "SAVEDATA_DIRECTORY")==0) {
                strncpy(sd->dir_name, (char*) data, sizeof(sd->dir_name));
            } else
            if(strcmp(key, "MAINTITLE")==0) {
                strncpy(sd->main_title, (char*) data, sizeof(sd->main_title));
            } else
            if(strcmp(key, "SUBTITLE")==0) {
                strncpy(sd->sub_title, (char*) data, sizeof(sd->sub_title));
            } else
            if(strcmp(key, "DETAIL")==0) {
                strncpy(sd->detail, (char*) data, sizeof(sd->detail));
            } else
            if(strcmp(key, "SAVEDATA_BLOCKS")==0) {
                sd->blocks = *((uint64_t*) data);
            } else
            if(strcmp(key, "TITLE_ID")==0) {
                strncpy(sd->title_id, (char*) data, sizeof(sd->title_id));
            } else
            if(strcmp(key, "ACCOUNT_ID")==0) {
                sd->account_id = *((uint64_t*) data);
            } else
            if(strcmp(key, "SAVEDATA_LIST_PARAM")==0) {
                sd->user_param = *((int*) data);
            } else
            if(strcmp(key, "PARAMS")==0) {
                sd->user_id = *((uint32_t*) (data+0x04));
                strncpy(sd->game_title_id,(char*) (data+0x3C), sizeof(sd->game_title_id));
                sd->last_dt = *((uint32_t*) (data+0x68));
            }
        }
    }

    free(sfo_hdr);
    free(sfo_meta);
    free(sfo_keys_name);
    free(sfo_keys_data);
    free(sfo_keys);
    close(fd);
    return 0;
}

int show_sd_sfo_info(char* file, savedata* sd) {

    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        printf_s("File '%s' not found\n", file);
        return 1;
    }

    // Get total size
    struct stat st;
    int sfo_file_size=0;

    fstat(fd, &st);
    sfo_file_size = st.st_size;

    sfo_header *sfo_hdr;
    sfo_index_metadata *sfo_meta;
    void* sfo_keys_name;
    void* sfo_keys_data;
    sfo_key* sfo_keys;


    sfo_hdr = malloc(sizeof(sfo_header));
    read(fd, sfo_hdr, sizeof(sfo_header));

    int size_meta = sizeof(sfo_index_metadata) * sfo_hdr->n_entries;
    sfo_meta = malloc(size_meta);
    read(fd, sfo_meta, size_meta);

    int size_keys_name = sfo_hdr->data_t_offset - sizeof(sfo_header) - size_meta;
    sfo_keys_name = malloc(size_keys_name);
    read(fd, sfo_keys_name, size_keys_name);

    int size_keys_data = sfo_file_size - sfo_hdr->data_t_offset;
    sfo_keys_data = malloc(size_keys_data);
    read(fd, sfo_keys_data, size_keys_data);

    sfo_keys = malloc(sizeof(sfo_key) * sfo_hdr->n_entries);

    for(int i=0; i<sfo_hdr->n_entries; i++) {
        sfo_key* key = &sfo_keys[i];
        key->name = (char*) sfo_keys_name + sfo_meta[i].key_offset;
        key->data_size = sfo_meta[i].param_length;
        key->data = (uint8_t*) sfo_keys_data + sfo_meta[i].data_offset;
    }

    printf_s("\n\n");
    printf_s("Index |         Name        |    Value\n");
    printf_s("---------------------------------------------------------------\n");
    for (int i=0; i<sfo_hdr->n_entries; i++) {

        char* key = sfo_keys[i].name;
        void* data = sfo_keys[i].data;
        uint32_t d_size = sfo_keys[i].data_size;

        if(sd) {
            if(strcmp(key, "SAVEDATA_DIRECTORY")==0) {
                strncpy(sd->dir_name, (char*) data, sizeof(sd->dir_name));
            } else
            if(strcmp(key, "MAINTITLE")==0) {
                strncpy(sd->main_title, (char*) data, sizeof(sd->main_title));
            } else
            if(strcmp(key, "SUBTITLE")==0) {
                strncpy(sd->sub_title, (char*) data, sizeof(sd->sub_title));
            } else
            if(strcmp(key, "DETAIL")==0) {
                strncpy(sd->detail, (char*) data, sizeof(sd->detail));
            } else
            if(strcmp(key, "SAVEDATA_BLOCKS")==0) {
                sd->blocks = *((uint64_t*) data);
            } else
            if(strcmp(key, "TITLE_ID")==0) {
                strncpy(sd->title_id, (char*) data, sizeof(sd->title_id));
            } else
            if(strcmp(key, "ACCOUNT_ID")==0) {
                sd->account_id = *((uint64_t*) data);
            } else
            if(strcmp(key, "SAVEDATA_LIST_PARAM")==0) {
                sd->user_param = *((int*) data);
            } else
            if(strcmp(key, "PARAMS")==0) {
                sd->user_id = *((uint32_t*) (data+0x04));
                strncpy(sd->game_title_id,(char*) (data+0x3C), sizeof(sd->game_title_id));
                sd->last_dt = *((uint32_t*) (data+0x68));
            }
        }

        printf_s("  %02d  | %-19s | ", i, key);
        if (sfo_meta[i].param_format == 0x204) {
            replace_char((char*) data, '\n', 'n');
            printf_s("'%s'", (char*) data);
        }
        else {
            printf_s("0x", data);
            int trunc = d_size > 16;
            int size = trunc ? 16 : d_size;
            for (int j=0; j<size; j++)
                printf_s("%02X", ((uint8_t*)data)[j]);
            if (trunc)
                printf_s(" (trunc) ");
        }
        printf_s("\n");    
    }
    printf_s("\n\n");

    free(sfo_hdr);
    free(sfo_meta);
    free(sfo_keys_name);
    free(sfo_keys_data);
    free(sfo_keys);
    close(fd);
    return 0;
}

int mount_sd_and_parse_sfo(void) {

    char selected_path[PATH_MAX];
    char selected_file[PATH_MAX];
    char option;
    int exit = 0;

    while (!exit) {
        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/             *** SD Mounter ***            /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Please select a user or the file explorer\n");
        printf_s("-------------------------------------------\n");
        
        printf_s("\n");
        printf_s(" [z] close\n\n");

        int n_users = show_users(1);

        if (n_users<=8)
            printf_s(" [%d] file explorer (default to %s)\n", n_users, IN_EX_PATH);
        else
            printf_s(" [%c] file explorer (default to %s)\n", 'a' + ((char)n_users-10), IN_EX_PATH);

        printf_s("\nSelection: ");

        option = read_char();

        // If we receive LF or CR we do nothing
        if(option==0x0A || option==0x0D)
            continue;

        if (option == 'z')
            return 1;
        if (option >= '0' && option <= '9') {
            option = option - '0';
        }
        else if (option >= 'a' && option <= 'y') {
            option = option - 'a';
        }

        if (option > (char) n_users)
            exit = 1;
        
        // File Explorer
        if (option == n_users) {
            file_explorer(IN_EX_PATH, "SaveData Parser", "Select a file to parse and show information", selected_file);
            if(!mount_prospero_sd(selected_file)) {
                show_sd_sfo_info(MOUNT_POINT_SFO, NULL);
                printf_s("SaveData file '%s' successfully mounted to " MOUNT_POINT "\n", selected_file);
                printf_s("Press any key to finish the sd mount and go back\n");
                option = read_char();
                unmount_prospero_sd();
                return 0;
            }
            //parse_sfo(selected_file);
            return 0;
        }

        // File Explorer on User Home
        if (option >= 0 && option < n_users) {
            sprintf(selected_path, "/user/home/%08x/savedata_prospero/", user_list[option].user_id);
            int ret = file_explorer(selected_path, "SaveData Parser", "Select App and File to parse and show information", selected_file);
            if (ret == 1) // Exit
                return 1;
            if(!mount_prospero_sd(selected_file)) {
                show_sd_sfo_info(MOUNT_POINT_SFO, NULL);
                printf_s("SaveData file '%s' successfully mounted to " MOUNT_POINT "\n", selected_file);
                printf_s("Press any key to finish the sd mount and go back\n");
                option = read_char();
                unmount_prospero_sd();
                return 0;
            }
            //parse_sfo(selected_file);
            return 0;
        }
        
    }
    return 1;
}

int resign_param_sfo(char* path, uint64_t account_id, uint32_t user_id) {
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf_s("Error opening param.sfp Error: %d %s\n", errno, strerror(errno));
        return 1;    
    }
    // Patch account_id
    int ret = lseek(fd, 0x1B8, SEEK_SET);
    if (ret == -1) {
        close(fd);
        printf_s("Error lseek param.sfp Error: %d %s\n", errno, strerror(errno));
        return 1;        
    }
    ret = write(fd, &account_id, sizeof(account_id));
    if (ret == -1) {
        close(fd);
        printf_s("Error writing new account_id to param.sfp Error: %d %s\n", errno, strerror(errno));
        return 1;        
    }
    // Patch user_id
    ret = lseek(fd, 0x660, SEEK_SET);
    if (ret == -1) {
        close(fd);
        printf_s("Error lseek param.sfp Error: %d %s\n", errno, strerror(errno));
        return 1;        
    }
    ret = write(fd, &user_id, sizeof(user_id));
    if (ret == -1) {
        close(fd);
        printf_s("Error writing new user_id to param.sfp Error: %d %s\n", errno, strerror(errno));
        return 1;        
    }
    close(fd);
    return 0;
}