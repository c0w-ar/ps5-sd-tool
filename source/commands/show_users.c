#include "show_users.h"

extern user user_list[16]; // Global at db.c

int show_users(int show_option) {
    int n_users = 0;
    printf_s("\n");
    for (int i=0 ; i<16 ; i++) {
        if (!user_list[n_users].name[0])
            continue;

        if (show_option) {
            if (i<=9)
                printf_s(" [%d] ID %08x '%-20s' PSN_ID %016lx\n", n_users, user_list[n_users].user_id, user_list[n_users].name, user_list[n_users].account_id);
            else
                printf_s(" [%c] ID %08x '%-20s' PSN_ID %016lx\n", 'a' + ((char)n_users-10), user_list[n_users].user_id, user_list[n_users].name, user_list[n_users].account_id);
        }
        else {
            printf_s(" [%02d] ID %08x '%-20s' PSN_ID %016lx\n", n_users, user_list[n_users].user_id, user_list[n_users].name, user_list[n_users].account_id);
        }
        n_users++;
    }
    printf_s("\n");
    return n_users;
}