
#include "sd.h"
#include "db.h"
#include "show_users.h"
#include "file_explorer.h"

extern user user_list[16]; // Global from show_users.c

int sd_mounted = 0; // Control mount point usage

int mount_prospero_sd(char* path) {
    int ret;

    pfsmgr_buffer pfsmgr_buffer;

    printf_s("Enter mount_prospero_sd: %s\n", path);

    int fd_sd = open(path, O_RDONLY, 0);
    if (fd_sd == -1) {
        printf_s("Error open sd_test Error: %d %s\n", errno, strerror(errno));
        return 1;
    }

    ret = pread(fd_sd, pfsmgr_buffer.encrypted_key, 0x60, 0x800); // Read psfSKKey at offset 0x800
    close(fd_sd);
    if (ret == -1) {
        printf_s("Error pread sd_test : %d Error: %d %s\n", ret, errno, strerror(errno));
        return 1;
    }

    int fd_pfsmgr = open("/dev/pfsmgr", 2);
    if (fd_pfsmgr == -1) {
        printf_s("Error open fd_pfsmgr Error: %d %s\n", errno, strerror(errno));
        return 1;    
    }

    // Decrypt key
    ret = ioctl(fd_pfsmgr, 0xc0845302, &pfsmgr_buffer);
    if (ret == -1) {
        printf_s("Error decrypting key : %d Error: %d %s\n", ret, errno, strerror(errno));
        close(fd_pfsmgr);
        return 1;
    }
    close(fd_pfsmgr);

    // Mount SaveData from File
    MountOpt mopt;
    memset(&mopt, 0, sizeof(MountOpt));

    sceFsInitMountSaveDataOpt(&mopt);
    mopt.budgetid = "system";
    ret = sceFsMountSaveData(&mopt, path, MOUNT_POINT, pfsmgr_buffer.key);
    printf_s("ret from mount: %d\n", ret);
    if (ret < 0) 
    {
        printf_s("Mount %s failed\n", path);
        return 1;
    }

    sd_mounted = 1;
    
    return 0;
}

int unmount_prospero_sd(void) {
    if (sd_mounted || 1) {
        UmountOpt uopt;
        sceFsInitUmountSaveDataOpt(&uopt);
        sceFsUmountSaveData(&uopt, MOUNT_POINT, 0, 0);
        sd_mounted = 0;
    }
    return 0;
}

int export_local_sd(void) {

    char selected_file[PATH_MAX];
    char option;
    int exit = 0;

    while (!exit) {
        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/            *** SD Exporter ***            /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Please select a user\n");
        printf_s("-------------------------------------------\n");
        
        printf_s("\n");
        printf_s(" [z] close\n\n");

        int n_users = show_users(1);

        printf_s("\nSelection: ");

        option = read_char();

        // If we receive LF or CR we do nothing
        if(option==0x0A || option==0x0D)
            continue;

        if (option == 'z')
            exit = 1;
        if (option >= '0' && option <= '9') {
            option = option - '0';
        }
        else if (option >= 'a' && option <= 'y') {
            option = option - 'a';
        }

        // Just exit
        if (option >= n_users)
            continue;
        
        // File Explorer on User Home
        if (option >= 0 && option < n_users) {
            exit = 0;
            while (!exit) {
                printf_s("\x1b[2J\x1b[H");    // Clear screen
                printf_s(" --------------------------------------------\n");
                printf_s("/            *** SD Exporter ***            /\n");
                printf_s("-------------------------------------------\n");
                printf_s(" Selected user: %08x - %16lx - %s\n", user_list[option].user_id, user_list[option].account_id, user_list[option].name);
                printf_s(" Please select a game\n");
                printf_s("-------------------------------------------\n");
                
                printf_s("\n");
                printf_s(" [z] close\n\n");

                game* g_options[24];
                game* g;
                int n_games = 0;
                int g_option;

                g = user_list[option].game_list;
                while(g) {
                    if (n_games<=9)
                        printf_s(" [%d] ", n_games);
                    else
                        printf_s(" [%c] ", 'a'+(char)n_games-10);

                    printf_s("%-9.9s - %s\n", g->id, g->name);
                    g_options[n_games] = g;
                    g = g->next;
                    n_games++;
                }

                printf_s("\nSelection: ");
                g_option = read_char();

                // If we receive LF or CR we do nothing
                if(g_option==0x0A || g_option==0x0D)
                    continue;

                if (g_option == 'z')
                    exit = 1;
                if (g_option >= '0' && g_option <= '9') {
                    g_option = g_option - '0';
                }
                else if (g_option >= 'a' && g_option <= 'y') {
                    g_option = g_option - 'a';
                }

                // Just do nothing
                if (g_option >= n_games)
                    continue;
                
                // File Explorer on User Home
                if (0 <= g_option < n_games) {
                    exit = 0;
                    while (!exit) {
                        printf_s("\x1b[2J\x1b[H");    // Clear screen
                        printf_s(" --------------------------------------------\n");
                        printf_s("/            *** SD Exporter ***            /\n");
                        printf_s("-------------------------------------------\n");
                        printf_s(" Selected user: %08x - %16lx - %s\n", user_list[option].user_id, user_list[option].account_id, user_list[option].name);
                        printf_s(" Selected game: %9s - %s\n", g_options[g_option]->id, g_options[g_option]->name);
                        printf_s(" Please select a savedata folder\n");
                        printf_s("-------------------------------------------\n");
                        
                        printf_s("\n");
                        printf_s(" [z] close\n\n");

                        savedata* sd_options[24];
                        savedata* sd;
                        int n_savedata = 0;
                        int n_option;

                        sd = g_options[g_option]->sd_list;
                        while(sd) {
                            if (n_savedata<=9)
                                printf_s(" [%d] ", n_savedata);
                            else
                                printf_s(" [%c] ", 'a'+(char)n_savedata-10);

                            printf_s("%-20.20s - %-20.20s - %s\n", sd->dir_name, sd->main_title, sd->last_time);
                            sd_options[n_savedata] = sd;
                            sd = sd->next;
                            n_savedata++;
                        }

                        printf_s("\nSelection: ");
                        n_option = read_char();

                        // If we receive LF or CR we do nothing
                        if(n_option==0x0A || n_option==0x0D)
                            continue;

                        if (n_option == 'z')
                            exit = 1;
                        if (n_option >= '0' && n_option <= '9') {
                            n_option = n_option - '0';
                        }
                        else if (n_option >= 'a' && n_option <= 'y') {
                            n_option = n_option - 'a';
                        }

                        // Just do nothing
                        if (n_option >= n_savedata)
                            continue;
                        
                        // File Explorer on User Home
                        if (n_option >= 0 && n_option < n_savedata) {
                            exit = 1;
                            char sd_path[PATH_MAX];
                            sprintf(sd_path, USER_SD_GAME_PATH, user_list[option].user_id, g_options[g_option]->sd_id, sd_options[n_option]->dir_name);
                            mount_prospero_sd(sd_path);
                            savedata sd;
                            //parse_sfo(MOUNT_POINT_SFO, &sd);    // Show data of SD to export
                            
                            printf_s("\nSavedata information: ");
                            show_sd_sfo_info(MOUNT_POINT_SFO, &sd);

                            printf_s("\nDo you want to export this SD? (y/n): ");
                            char y_n;
                            exit = 0;
                            while (!exit) {
                                y_n = read_char();
                                if (y_n == 'y' || y_n == 'Y') {
                                    // Export
                                    char exp_path[PATH_MAX];
                                    sprintf(exp_path, IN_EX_GAME_FOLDER_PATH, g_options[g_option]->sd_id, sd_options[n_option]->dir_name);
                                    recursive_mkdir(exp_path);
                                    copy_recursive(MOUNT_POINT, exp_path);
                                    printf_s("\n Savedata successfully exported to: %s\n\nPress any key to continue", exp_path);
                                    exit = 1;
                                    read_char();
                                }
                                if (y_n == 'n' || y_n == 'N') {
                                    exit = 1;
                                }
                            }
                            unmount_prospero_sd();

                        }
                        return 0;
                    }
                }
                return 0;
            }
        }
        
    }
    return 1;
}

int import_extern_sd(void) {

    char selected_file[PATH_MAX];
    char option;
    int exit = 0;

    while (!exit) {
        printf_s("\x1b[2J\x1b[H");    // Clear screen
        printf_s(" --------------------------------------------\n");
        printf_s("/            *** SD Importer ***            /\n");
        printf_s("-------------------------------------------\n");
        printf_s(" Please select a user\n");
        printf_s("-------------------------------------------\n");
        
        printf_s("\n");
        printf_s(" [z] close\n\n");

        int n_users = show_users(1);

        printf_s("\nSelection: ");

        option = read_char();

        // If we receive LF or CR we do nothing
        if(option==0x0A || option==0x0D)
            continue;

        if (option == 'z')
            exit = 1;
        if (option >= '0' && option <= '9') {
            option = option - '0';
        }
        else if (option >= 'a' && option <= 'y') {
            option = option - 'a';
        }

        // Just exit
        if (option >= n_users)
            continue;
        
        // Folder Explorer
        if (option >= 0 && option < n_users) {
            user* selected_user = &user_list[option];
            char message[200];
            char sd_folder[PATH_MAX];

            sprintf(message, "User: %08x - %016lx - %s", user_list[option].user_id, user_list[option].account_id, user_list[option].name);

            int ret = folder_explorer(IN_EX_PATH, "SD Importer", message, sd_folder);
            if(ret) {
                printf_s("No savedata folder was selected. Please press a key to try again\n");
                read_char();
                exit = 1;
                break;
            }

            char param_sfo[PATH_MAX];
            savedata sd;
            strncpy(param_sfo, sd_folder, sizeof(param_sfo));
            append_and_clean_path(param_sfo, "sce_sys/param.sfo");
            
            ret = show_sd_sfo_info(param_sfo, &sd);
            if(ret) {
                printf_s("Seems like the selected folder is not a savedata folder. Please press a key to continue\n");
                read_char();
                exit = 1;
                break;
            }

            char sd_tmp_path[PATH_MAX];
            sprintf(sd_tmp_path,"%s/%s", IN_EX_PATH, sd.dir_name); // Use savedata dir_name from param.sfo 

            ret = create_and_mount_sd(sd_tmp_path, sd.blocks*0x40*0x400);
            if (ret) {
                printf_s("There was a problem while creating the encrypted savedata file. Pleese press a key to continue\n");
                read_char();
                exit = 1;
                break;
            }

            // Now copy everything from source folder to mounted SD
            copy_recursive(sd_folder, MOUNT_POINT);

            if(sd.account_id != selected_user->account_id) {
                printf_s("The selected savedata folder has a different user_id or account_id (PSN) (sd: 0x%08x {0x%016lx} - user: 0x%08x {0x%016lx}).\nIt is going to be resigned before importing.\nPress any key to continue.\n", sd.user_id, sd.account_id, selected_user->user_id, selected_user->account_id);
                read_char();

                ret = resign_param_sfo(MOUNT_POINT_SFO, selected_user->account_id, selected_user->user_id);
                if (ret) {
                    printf_s("There was a problem while signinng the encrypted savedata file. Pleese press a key to continue\n");
                    read_char();
                    unmount_prospero_sd();
                    // Delete the tmp sd file
                    unlink(sd_tmp_path);
                    exit = 1;
                    break;
                }
            }

            //Check if savedata already exists for this user
            ret = check_if_sd_exists(selected_user->user_id, sd.title_id, sd.dir_name);
            if (ret) {
                printf_s("The user already has a savadata with the same name. The savedata is going to be overwritten. Do you wish to continue? (y/n): ");
                int yes_no = read_char();
                if(yes_no != 'y') {
                    printf_s("Savedata import aborted. Pleese press a key to continue\n");
                    read_char();
                    unmount_prospero_sd();
                    // Delete the tmp sd file
                    unlink(sd_tmp_path);
                    exit = 1;
                    break;  
                }
                else {
                    ret = update_sd(selected_user, &sd);
                }
            }
            else {
                printf_s("You are going to add a new savedata to the user. Do you wish to continue? (y/n): ");
                int yes_no = read_char();
                if(yes_no != 'y') {
                    printf_s("Savedata import aborted. Pleese press a key to continue\n");
                    read_char();
                    unmount_prospero_sd();
                    // Delete the tmp sd file
                    unlink(sd_tmp_path);
                    exit = 1;
                    break;  
                }
                else {
                    ret = insert_sd(selected_user, &sd);
                }
            }

            char icon_src[PATH_MAX];
            char icon_dst[PATH_MAX];
            char sd_dst[PATH_MAX];

            sprintf(icon_src, "%s/%s", MOUNT_POINT, "sce_sys/icon0.png");
            sprintf(icon_dst, USER_SD_GAME_META_PATH_PNG, selected_user->user_id, sd.title_id, sd.dir_name);
            sprintf(sd_dst, USER_SD_GAME_PATH, selected_user->user_id, sd.title_id, sd.dir_name);

            // Create needed directories if they don't exists (new local users)
            if(recursive_mkdir(icon_dst)) {
                printf_s("Failed to create needed directory: '%s'\n Pleese press a key to continue\n", icon_dst);
                read_char();
                unmount_prospero_sd();
                // Delete the tmp sd file
                unlink(sd_tmp_path);
                exit = 1;
                break;  
            }

            if(recursive_mkdir(sd_dst)) {
                printf_s("Failed to create needed directory: '%s'\n Pleese press a key to continue\n", sd_dst);
                read_char();
                unmount_prospero_sd();
                // Delete the tmp sd file
                unlink(sd_tmp_path);
                exit = 1;
                break;  
            }

            // Copy out SD icon
            copy_file(icon_src, icon_dst);
            // Now the new savedata file has everything inside
            unmount_prospero_sd();
            // Copy out SD file
            copy_file(sd_tmp_path,sd_dst);
            // Delete the tmp SD file
            unlink(sd_tmp_path);

            printf_s("Savedata file imported successfully\n Pleese press a key to continue\n");
            read_char();
        }
        
    }
    return 1;
}

int create_and_mount_sd(char* path, uint64_t size) {
    
    int ret;
    pfsmgr_buffer pfsmgr_buffer;

    // Delete old file if it exists
    if (access(path, F_OK) == 0) {
        unlink(path);
    }
    
    int fd_pfsmgr = open("/dev/pfsmgr", 2);
    if (fd_pfsmgr == -1) {
        printf_s("Error open fd_pfsmgr Error: %d %s\n", errno, strerror(errno));
        return 1;    
    }

    // Create key
    ret = ioctl(fd_pfsmgr, 0x40845303, &pfsmgr_buffer);
    if (ret < 0) {
        printf_s("Error creating key: %d Error: %d %s\n", ret, errno, strerror(errno));
        close(fd_pfsmgr);
        return 1;
    }

    // Decrypt key
    ret = ioctl(fd_pfsmgr, 0xc0845302, &pfsmgr_buffer);
    if (ret < 0) {
        printf_s("Error decrypting key: %d Error: %d %s\n", ret, errno, strerror(errno));
        close(fd_pfsmgr);
        return 1;
    }
    close(fd_pfsmgr);

    CreateOpt copt;
    memset(&copt, 0, sizeof(copt));
    sceFsInitCreatePprPfsSaveDataOpt(&copt);
    copt.flags[1] = 0x02;

    ret = sceFsCreatePprPfsSaveDataImage(&copt, path, 0, size, pfsmgr_buffer.key);
    if (ret < 0) {
        printf_s("Format failed: %d Error: %d %s\n", ret, errno, strerror(errno));
        return 1;
    }

    MountOpt mopt;
    memset(&mopt, 0, sizeof(MountOpt));

    sceFsInitMountSaveDataOpt(&mopt);
    mopt.budgetid = "system";
    ret = sceFsMountSaveData(&mopt, path, MOUNT_POINT, pfsmgr_buffer.key);
    if (ret < 0) 
    {
        printf_s("Mount failed: %d Error: %d %s\n", ret, errno, strerror(errno));
        return 1;
    }
    return 0;
}

int resign_sd(void) {
    char selected_path[PATH_MAX];
    char selected_file[PATH_MAX];
    char option;
    int exit = 0;


    file_explorer(IN_EX_PATH, "Savedata Resigner", "Select a savedata file to resign", selected_file);
    if(!mount_prospero_sd(selected_file)) {
        savedata sd;
        int ret = show_sd_sfo_info(MOUNT_POINT_SFO, &sd);
        if (ret) {
            unmount_prospero_sd();
            printf_s("Seems like the selected savedata file has no SFO\n");
            printf_s("Press any key to finish and go back\n");
            option = read_char();
            return 1;
               
        }
        printf_s("Savedata file '%s' successfully mounted to " MOUNT_POINT "\n", selected_file);

        printf_s("Please input the new account_id (PSN) in hex format (0xAABBCCDDEEFFGGHH) : ");
        uint64_t new_account_id = read_uint64();

        printf_s("Please input the new user_id (local user) in hex format (0xAABBCCDD) : ");
        uint32_t new_user_id = read_uint32();

        printf_s("You are going to resign the savedata.\n");
        printf_s("Current information - user_id : 0x%08x account_id : 0x%016lx\n", sd.user_id, sd.account_id);
        printf_s("New information - user_id : 0x%08x account_id : 0x%016lx\n", new_user_id, new_account_id);

        printf_s("\nDo you wish to continue? (y/n): ");
        int yes_no = read_char();
        if(yes_no != 'y') {
            printf_s("Savedata resign aborted. Pleese press a key to continue\n");
            read_char();
            unmount_prospero_sd();
            return 1;
        }
        
        ret = resign_param_sfo(MOUNT_POINT_SFO, new_account_id, new_user_id);
        if (ret) {
            printf_s("Something went wrong while savedata resigning. Pleese press a key to continue\n");
            read_char();
            unmount_prospero_sd();
            return 1;
        }
        printf_s("Savedata successfully resigned. Pleese press a key to continue\n");
        read_char();
        unmount_prospero_sd();
        return 0;
    }
    printf_s("Savedata file '%s' could not be mounted\n");
    printf_s("Pleese press a key to continue\n");
    option = read_char();
    return 1;
}