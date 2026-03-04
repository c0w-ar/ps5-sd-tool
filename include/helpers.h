#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <stdarg.h>

#define SIZE_user_id        4
#define SIZE_user_name      17
#define SIZE_account_id     8
#define SIZE_passcode       17
#define SIZE_NP_env         17
#define SIZE_login_flag     4

#define KEY_user_id(a)      get_entity_number(a, 16, 65536, 125829376, 127140096)
#define KEY_user_name(a)    get_entity_number(a, 16, 65536, 125829632, 125829120)
#define KEY_login_flag(a)   get_entity_number(a, 16, 65536, 125831168, 125829120)
#define KEY_account_id(a)   get_entity_number(a, 16, 65536, 125830400, 125829120)
#define KEY_NP_env(a)       get_entity_number(a, 16, 65536, 125874183, 125874176)



int sceRegMgrGetStr (int, char*, int);
int sceRegMgrGetBin (int, void*, int);
int sceRegMgrGetInt (int, uint32_t*);

typedef struct notify_request {
  char useless1[45];
  char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);

static int get_entity_number (int a, int b, int c, int d, int e);
int reg_get_user_id (int user_index, uint32_t* user_id);
int reg_get_username (int user_index, char* username);
int reg_get_account_id (int user_index, uint64_t* psn_account_id);

void replace_char(char *s, char original, char new);
char* remove_last_path_entry(char *path);
int recursive_mkdir(const char *path);
void copy_file(const char *src, const char *dst);
void copy_recursive(const char *src, const char *dst);

void notify(const char *fmt, ...);

#endif