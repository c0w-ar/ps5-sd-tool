#ifndef __SAVEDATA_FILE_H__
#define __SAVEDATA_FILE_H__

#include <stdint.h>

typedef struct _sd_file {
    uint8_t header[0x5A0];
} sd_file;

typedef struct _sd_header {
    uint8_t unkwnown[0x380];
    uint8_t rootICV[0x20];
} sd_header;

typedef struct _sd_header2 {
    uint64_t magic; // 0x459a9bbbc11a2b79
    uint32_t version;
    uint32_t unknown;
    uint8_t  iv[0x10];
    //uint8_t  auth_code[0x50];
    // The following 0x50 bytes are encrypted
    uint8_t  unknown2[0x20];
    uint8_t  rootICV[0x20];
    uint8_t  unknown3[0x10];
} sd_header2;
#endif
