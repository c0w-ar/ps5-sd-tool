#include "helpers.h"

static int get_entity_number(int a, int b, int c, int d, int e)
{
    if (a < 1 || a > b)
    {
        return e;
    }
    return (a - 1) * c + d;
}

int reg_get_user_id (int user_index, uint32_t* user_id)
{
    return sceRegMgrGetInt(KEY_user_id(user_index), user_id);
}
int reg_get_username (int user_index, char* username)
{
    username[0] = 0;
    return sceRegMgrGetStr(KEY_user_name(user_index), username, SIZE_user_name);
}

int reg_get_account_id (int user_index, uint64_t* psn_account_id)
{
    return sceRegMgrGetBin(KEY_account_id(user_index), psn_account_id, SIZE_account_id);
}

void replace_char(char *s, char original, char new) {
    char *p;
    while ((p = strchr(s, original)) != NULL) {
        *p = new;
    }
}

char* remove_last_path_entry(char *path) {
    if (!path) return NULL;
    char *last_slash = strrchr(path, '/');
    char *prev_dir = NULL;
    if (last_slash != NULL) {
        size_t len = (size_t)(last_slash - path);
        if (len==0) len = 1;
        prev_dir = malloc(len + 1);
        memcpy(prev_dir, path, len);
        prev_dir[len] = '\0';
    }
    return prev_dir;
}

int recursive_mkdir(const char *path) {
    char temp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(temp, sizeof(temp), "%s", path);

    // Remove filename if provided
    char *dir = dirname(temp);

    len = strlen(dir);

    for (p = dir + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(dir, 0777) != 0) {
                if (errno != EEXIST) {
                    return -1; 
                }
            }
            *p = '/';
        }
    }

    if (mkdir(dir, 0777) != 0) {
        if (errno != EEXIST) {
            return -1;
        }
    }
    return 0;
}

void copy_file(const char *src, const char *dst) {
    int fd_in = open(src, O_RDONLY);
    if (fd_in < 0) {
        return;
    }

    int fd_out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd_out < 0) {
        close(fd_in);
        return;
    }

    char buffer[8192];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(fd_in, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(fd_out, buffer, bytes_read);
        
        if (bytes_written != bytes_read) {
            break;
        }
    }

    close(fd_in);
    close(fd_out);
}

void copy_recursive(const char *src, const char *dst) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char src_full[1024];
    char dst_full[1024];

    if (stat(src, &statbuf) != 0) return;

    if (S_ISDIR(statbuf.st_mode)) {

        mkdir(dst, statbuf.st_mode);

        if (!(dir = opendir(src))) return;

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(src_full, sizeof(src_full), "%s/%s", src, entry->d_name);
            snprintf(dst_full, sizeof(dst_full), "%s/%s", dst, entry->d_name);
            copy_recursive(src_full, dst_full);
        }
        closedir(dir);
    } else {

        copy_file(src, dst);
    }
}

void notify(const char *fmt, ...) {
  notify_request_t req;
  va_list args;

  bzero(&req, sizeof req);
  va_start(args, fmt);
  vsnprintf(req.message, sizeof req.message, fmt, args);
  va_end(args);

  sceKernelSendNotificationRequest(0, &req, sizeof req, 0);
}