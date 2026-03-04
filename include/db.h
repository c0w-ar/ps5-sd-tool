
#ifndef __DB_H__
#define __DB_H__

#include <unistd.h>

#define APP_DB "/system_data/priv/mms/app.db"

#define SQL_APPS \
"SELECT " \
"titleId, " \
"titleName " \
"FROM tbl_contentinfo " \
"WHERE titleId LIKE 'PPSA\%' " \
"ORDER BY titleId;"

#define APPINFO_DB "/system_data/priv/mms/appinfo.db"

#define SQL_APPINFO \
"SELECT " \
"titleId, " \
"MIN(CASE WHEN key = 'TITLE' THEN val END) AS name, " \
"COALESCE(MIN(CASE WHEN key = 'INSTALL_DIR_SAVEDATA' THEN val END), titleId) AS sd_dir " \
"FROM tbl_appinfo " \
"WHERE titleId LIKE 'PPSA\%' " \
"GROUP BY 1 " \
"ORDER BY 1;"

// Title ID is the name of the Base Game (e.g.: GTA V) and Game Title ID is the Located Game (like EU, USA, etc.)
#define USER_SD_DB_PATH "/system_data/savedata_prospero/%08x/db/user/savedata.db"

#define SQL_USER_SAVEDATA \
"SELECT " \
"game_title_id, " \
"title_id, " \
"dir_name, " \
"main_title, " \
"sub_title, " \
"mtime, " \
"account_id, " \
"user_id " \
"FROM savedata " \
"ORDER BY title_id, dir_name;"

#define SQL_USER_SAVEDATA_CHECK \
"SELECT EXISTS( SELECT 1 " \
"FROM savedata " \
"WHERE title_id LIKE '%%%s%%' AND dir_name LIKE '%%%s%%' " \
");"

#define SQL_USER_SAVEDATA_UPDATE \
"UPDATE savedata SET " \
"main_title = '%s', sub_title = '%s', detail = '%s', user_param = %d, " \
"mtime = strftime('%%Y-%%m-%%dT%%H:%%M:%%S.00Z', CURRENT_TIMESTAMP), " \
"blocks = %ld, free_blocks = %ld, size_kib = %ld " \
"WHERE title_id LIKE '%%%s%%' AND dir_name LIKE '%%%s%%';"

#define SQL_USER_SAVEDATA_INSERT \
"INSERT INTO savedata( " \
"title_id,      game_title_id,  dir_name ,      main_title, " \
"sub_title,     detail,         tmp_dir_name,   is_broken, " \
"user_param,    blocks,         free_blocks,    size_kib, " \
"mtime,         fake_broken,    account_id,     user_id, " \
"faked_owner,   cloud_icon_url, cloud_revision) " \
"VALUES ( " \
"'%s',          '%s',           '%s',           '%s', " \
"'%s',          '%s',           '',             0, " \
"%d,            %ld,            %ld,            %ld, " \
"strftime('%%Y-%%m-%%dT%%H:%%M:%%S.00Z', CURRENT_TIMESTAMP), 0, %ld, %d, " \
"0,             '',             0" \
");"

#define SQL_CREATE_SAVEDATA_TABLE \
"CREATE TABLE savedata (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, " \
"title_id NOT NULL, dir_name NOT NULL, main_title NOT NULL, sub_title , detail , " \
"tmp_dir_name , is_broken , user_param , blocks , free_blocks , size_kib , " \
"mtime NOT NULL, fake_broken , account_id , user_id , faked_owner , cloud_icon_url , " \
"cloud_revision , game_title_id NOT NULL, system_blocks);"
#define SQL_CREATE_SAVEDATA_TABLE_INDEX \
"CREATE INDEX savedata_index on savedata(title_id, dir_name, user_param, size_kib, mtime, game_title_id);"

typedef struct _savedata {
    char              dir_name[100];    // SAVEDATA_DIRECTORY
    char              main_title[200];  // MAINTITLE
    char              sub_title[200];   // SUBTITLE
    char              detail[200];      // DETAIL
    char              last_time[24];    // -> used when reading savedata.db
    uint32_t          last_dt;          // IN PARAMS offset 0x68 -> used when reading param.sfo
    uint64_t          blocks;           // SAVEDATA_BLOCKS
    int               user_param;       // SAVEDATA_LIST_PARAM
    char              title_id[10];     // TITLE_ID
    char              game_title_id[10];// IN PARAMS offset 0x2C
    uint64_t          account_id;       // ACCOUNT_ID               // We need to patch this
    uint32_t          user_id;          // IN PARAMS offset 0x04    // We need to patch this
    struct _savedata* next;
} savedata;

typedef struct _game {
    char           id[10];
    char           sd_id[10];   // Some Apps use a different ID for "Main ID" and "Game ID"
    char           name[200];
    savedata*      sd_list;
    struct _game*  next;
} game;

typedef struct _user {
    char         name[0x20];
    uint32_t     user_id;
    uint64_t     account_id;
    game* game_list;
} user;

int get_games(void);
savedata* get_game_last_savedata(game* game);
game* get_user_last_game(user* user);
void add_savedata_to_game(game* game, savedata* savedata);
void free_savedata_list(game* game);
void add_game_to_user(user* user, game* game);
void free_game_list(user* user);
void update_list_user_games_and_savedata(void);
int check_if_sd_exists(uint32_t user_id, char* title_id, char* dir_name);
int update_sd(user* user, savedata* sd);
int insert_sd(user* user, savedata* sd);
int check_or_create_sd_db(char* db_path);

#endif