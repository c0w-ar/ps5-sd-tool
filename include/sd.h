#ifndef __SD_H__
#define __SD_H__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ps5/kernel.h>
#include "client.h"
#include "parse_sfo.h"

#define MOUNT_POINT "/data/mount_sd"
#define MOUNT_POINT_SFO MOUNT_POINT "/sce_sys/param.sfo"
#define IN_EX_PATH "/mnt/usb0/PS5/ps5-sd-tool"
#define IN_EX_GAME_PATH IN_EX_PATH "/%9s"
#define IN_EX_GAME_FOLDER_PATH IN_EX_GAME_PATH "/%s"

#define USER_SD_MAIN_PATH "/user/home/%08x/savedata_prospero"
#define USER_SD_META_PATH "/user/home/%08x/savedata_prospero_meta/user"

#define USER_SD_GAME_MAIN_PATH USER_SD_MAIN_PATH "/%9s"
#define USER_SD_GAME_META_PATH USER_SD_META_PATH "/%9s"

#define USER_SD_GAME_PATH USER_SD_GAME_MAIN_PATH "/sdimg_%s"
#define USER_SD_GAME_META_PATH_PNG USER_SD_GAME_META_PATH "/%s_icon0.png"
#define USER_SD_GAME_META_PATH_SFO USER_SD_GAME_META_PATH "/sce_bu_%s.sfo"


typedef struct { int blockSize; uint8_t flags[2]; } CreateOpt;
typedef struct { uint8_t reserved; char *budgetid; } MountOpt;
typedef struct { uint8_t dummy; } UmountOpt;

int sceFsInitCreatePfsSaveDataOpt(CreateOpt *opt);
int sceFsInitCreatePprPfsSaveDataOpt(CreateOpt *opt);
int sceFsCreatePfsSaveDataImage(CreateOpt *opt, const char *path, int x, uint64_t size, uint8_t *key);
int sceFsCreatePprPfsSaveDataImage(CreateOpt *opt, const char *path, int x, uint64_t size, uint8_t *key);
int sceFsUfsAllocateSaveData(int fd, uint64_t size, uint64_t flags, int ext);
int sceFsInitMountSaveDataOpt(MountOpt *opt);
int sceFsMountSaveData(MountOpt *opt, const char *path, const char *mount, uint8_t *key);
int sceFsInitUmountSaveDataOpt(UmountOpt *opt);
int sceFsUmountSaveData(UmountOpt *opt, const char *mount, int handle, int ignore);

typedef struct _pfsmgr_buffer {
    uint8_t     encrypted_key[0x60];
    uint8_t     key[0x20];
    uint32_t    result;
} pfsmgr_buffer;

int mount_prospero_sd(char* path);
int unmount_prospero_sd(void);
int create_and_mount_sd(char* path, uint64_t size);
int export_local_sd(void);
int import_extern_sd(void);
int resign_sd(void);

#endif