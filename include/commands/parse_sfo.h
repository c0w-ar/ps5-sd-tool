#ifndef __PARSE_SFO_H__
#define __PARSE_SFO_H__

#include "sd.h"
#include "db.h"

void parse_sfo(char* file, savedata* sd);
int parse_sfo_info(char* file, savedata* sd);
int mount_sd_and_parse_sfo(void);
int show_sd_sfo_info(char* file, savedata* sd);
int resign_param_sfo(char* path, uint64_t account_id, uint32_t user_id);

#endif