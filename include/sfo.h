#ifndef __SFO_FILE_H__
#define __SFO_FILE_H__

#include <stdint.h>

typedef struct _sfo_header {
    uint32_t magic;         // 0x46535000
    uint32_t version;
    uint32_t key_t_offset;
    uint32_t data_t_offset;
    uint32_t n_entries;
} sfo_header;

typedef struct _sfo_index_metadata {
	uint16_t key_offset;
	uint16_t param_format;
	uint32_t param_length;
	uint32_t param_max_length;
	uint32_t data_offset;
} sfo_index_metadata;

typedef struct _sfo_key {
    char*    name;
    uint8_t* data;
    uint32_t data_size;
} sfo_key;


#endif
