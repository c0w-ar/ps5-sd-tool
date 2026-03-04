#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "sfo.h"
#include <string.h>
#include <limits.h>
#include <ps5/kernel.h>
#include "db.h"
#include "client.h"
#include "file_explorer.h"
#include "commands.h"
#include "sd.h"

char selected_path[PATH_MAX];

extern user user_list[16];  // Global from db.c
extern int sd_mounted;      // Control mount point usage

int main(int argc, char *argv[]) {

    pid_t mypid = getpid();
    uint64_t curr_authid;
    uint64_t new_authid = 0x4800000000000010;

    if(!(curr_authid = kernel_get_ucred_authid(mypid))) {
        printf("kernel_get_ucred_authid failed\n");
        return -1;
    }
    
    if(kernel_set_ucred_authid(mypid, new_authid)) {
        printf("kernel_set_ucred_authid failed\n");
        return -1;
    }

    // Create needed directories
    if(recursive_mkdir(MOUNT_POINT)) {
        printf("Failed to create dir for sd mount\n");
        return -1;
    }

    if(recursive_mkdir(IN_EX_PATH)) {
        printf("Failed to create dir for Import/Export on usb0\n");
        return -1; 
    }

    get_games();

    update_list_user_games_and_savedata();

    //return 0;

    char selected_file[PATH_MAX];
    memset(selected_path, 0, PATH_MAX);
    strcpy(selected_path, "/user/home");

    char option;
    int exit = 0;

    wait_client(); // Wait for the client connection

    while (!exit) {
        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/           *** PS5 Save Tool ***           /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Detected Users:\n");
        show_users(0);
        printf_s(" Please select an option\n");
        printf_s("-------------------------------------------\n");
        printf_s("\n");
        printf_s(" [z] close\n\n");
        printf_s(" [0] mount game savedata file and show info\n");
        printf_s(" [1] resign game savadata\n");
        printf_s(" [2] import game savadata (from usb0)\n");
        printf_s(" [3] export game savadata (to usb0)\n");
        

        printf_s("\nSelection: ");
        option = read_char();

        if(option==0x0A || option==0x0D)
            continue;

        switch (option) {
            case 'z':
                exit = 1;
                break;
            // If we receive LF or CR we do nothing
            case 0x0A:
            case 0x0D:
                continue;
            case '0':
                mount_sd_and_parse_sfo();
                break;
            case '1':
                resign_sd();
                break;
            case '2':
                import_extern_sd();
                break;
            case '3':
                export_local_sd();
                break;
            case '4':
                unmount_prospero_sd();
                break;
        }

        if (!exit) {
            printf_s("Command finished\n");
            printf_s("Press any key to go back\n");
            option = read_char();
        }
    }

    close_client();
    return 0;
}