#include "db.h"
#include "helpers.h"
#include "client.h"
#include "sqlite3.h"

user user_list[16];
game* game_list;

void get_users(void) {
    int n_users = 0;
    printf_s("\n");
    for (int i=0 ; i<16 ; i++) {
        char name[0x20];

        reg_get_username(i, name);
        if (!name[0])
            continue;
        
        // Only added if User Name is not null
        reg_get_username(i, user_list[n_users].name);
        reg_get_user_id(i, &user_list[n_users].user_id);
        reg_get_account_id(i, &user_list[n_users].account_id);

        n_users++;
    }
    printf_s("\n");
}

int check_or_create_sd_db(char* db_path) {
    sqlite3 *db;
    char *err_msg = 0;
    
    // Create all directory structure
    recursive_mkdir(db_path);

    int exists = access(db_path, F_OK);

    int ret = sqlite3_open(db_path, &db); // DB is create if it does not exist
    if (ret != SQLITE_OK) {
        printf("Could not create savedata db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    if (exists != 0) {
        printf("Exect create table: '%s'\n", SQL_CREATE_SAVEDATA_TABLE);
        ret = sqlite3_exec(db, SQL_CREATE_SAVEDATA_TABLE, NULL, 0, &err_msg);
        if (ret != SQLITE_OK) {
            printf("Error creating table in savedata db: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
        printf("Exect create index: '%s'\n", SQL_CREATE_SAVEDATA_TABLE_INDEX);
        ret = sqlite3_exec(db, SQL_CREATE_SAVEDATA_TABLE_INDEX, NULL, 0, &err_msg);
        if (ret != SQLITE_OK) {
            printf("Error creating table in savedata db: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
    }

    sqlite3_close(db);
    return 0;
}

int check_if_sd_exists(uint32_t user_id, char* title_id, char* dir_name) {

    char db_path[PATH_MAX];
    sprintf(db_path, USER_SD_DB_PATH, user_id);
    char sql[400];
    sprintf(sql, SQL_USER_SAVEDATA_CHECK, title_id, dir_name);

    sqlite3 *db;
    sqlite3_stmt *res;
    int ret;
    char *err_msg = 0;
    int exists = 0;

    // First check if savedata db exists if not, create it
    ret = check_or_create_sd_db(db_path);

    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    ret = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (ret == SQLITE_OK) {
        if (sqlite3_step(res) == SQLITE_ROW) {
            exists = sqlite3_column_int(res, 0);
        }
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return exists;
}

int update_sd(user* user, savedata* sd) {
    char db_path[PATH_MAX];
    sprintf(db_path, USER_SD_DB_PATH, user->user_id);
    char sql[400];

    replace_char((char*) sd->main_title, '\'', ' ');

    sprintf(sql, SQL_USER_SAVEDATA_UPDATE,
        sd->main_title, sd->sub_title, sd->detail, sd->user_param,
        sd->blocks, sd->blocks, sd->blocks*0x40,
        sd->title_id, sd->dir_name
    );

    sqlite3 *db;
    int ret;
    char *err_msg = 0;

    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    ret = sqlite3_exec(db, sql, NULL, 0, &err_msg);
    if (ret != SQLITE_OK) {
        printf("Something went wrong when trying to insert a new record: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}

int insert_sd(user* user, savedata* sd) {
    
    char db_path[PATH_MAX];
    sprintf(db_path, USER_SD_DB_PATH, user->user_id);
    char sql[800];

    replace_char((char*) sd->main_title, '\'', ' ');

    sprintf(sql, SQL_USER_SAVEDATA_INSERT,
        sd->title_id, sd->game_title_id, sd->dir_name, sd->main_title,
        sd->sub_title, sd->detail,
        sd->user_param, sd->blocks, sd->blocks, sd->blocks*0x40,
        user->account_id, user->user_id
    );

    sqlite3 *db;
    int ret;
    char *err_msg = 0;

    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    ret = sqlite3_exec(db, sql, NULL, 0, &err_msg);
    if (ret != SQLITE_OK) {
        printf("Something went wrong when trying to insert a new record: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}

int get_games(void) {
    
    game** g = &game_list;

    sqlite3 *db;
    sqlite3_stmt *res;
    int ret;

    ret = sqlite3_open(APPINFO_DB, &db);
    if (ret != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    ret = sqlite3_prepare_v2(db, SQL_APPINFO, -1, &res, 0);
    if (ret == SQLITE_OK) {
        while (sqlite3_step(res) == SQLITE_ROW) {
            *g = malloc(sizeof(game));
            memset(*g, 0, sizeof(game));

            const char *tid = (const char*)sqlite3_column_text(res, 0);
            if (tid) strncpy((*g)->id, tid, sizeof((*g)->id));

            const char *tname = (const char*)sqlite3_column_text(res, 1);
            if (tname) strncpy((*g)->name, tname, sizeof((*g)->name));

            const char *tsd_id = (const char*)sqlite3_column_text(res, 2);
            if (tsd_id) strncpy((*g)->sd_id, tsd_id, sizeof((*g)->sd_id));

            printf("Game ID: %s - SD ID: %s - Title: %s\n", (*g)->id, (*g)->sd_id, (*g)->name);
            g = &((*g)->next);
        }
    }
    sqlite3_finalize(res);
    sqlite3_close(db);

    if (game_list==NULL)
        return 1;
    else
        return 0;
}

char* get_game_title_from_id(char* g_id) {
    game* g;

    // Update Console Game List
    if (game_list==NULL)
        if(get_games())
            return NULL;
    
    g = game_list;

    while(strcmp(g->id, g_id) != 0) {
        g=g->next;
        // Avoid strcmp to NULL pointer
        if (!g)
            break;
    }
    if (g)
        return g->name;
    else
        return NULL;
}

void sanitize_string(char *str) {
    while (*str) {
        if ((unsigned char)*str > 127) {
            *str = ' ';
        }
        str++;
    }
}

void update_game_title (game* g, char* id) {
    char* game_name = get_game_title_from_id(id);
    if (game_name) {
        strcpy(g->name, game_name);
        sanitize_string(g->name);
    }
    else
        strcpy(g->name, "Not Found"); 
}

game* get_user_games(int user_index) {
    user* user = &user_list[user_index];
    char db_path[PATH_MAX];
    sqlite3 *db;
    sqlite3_stmt *res;
    int ret;
    game* g;
    game* g_head;
    savedata* sd;
    char curr_g_id[10]={0};

    // Update Console Game List
    if (game_list==NULL)
        get_games();

    sprintf(db_path, USER_SD_DB_PATH, user->user_id);

    // First check if savedata db exists if not, create it
    ret = check_or_create_sd_db(db_path);

    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf_s("Cannot open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    ret = sqlite3_prepare_v2(db, SQL_USER_SAVEDATA, -1, &res, 0);
    if (ret == SQLITE_OK) {

        printf("---------------------------------------------------------------------------------------------------------------------\n");
        printf("|   GAME ID   |          GAME        |          SD Directory          |        Title         |       Subtitle       |\n");
        printf("---------------------------------------------------------------------------------------------------------------------\n");
        while (sqlite3_step(res) == SQLITE_ROW) {

            const char *t_id = (const char*)sqlite3_column_text(res, 0);
            const char *t_sd_id = (const char*)sqlite3_column_text(res, 1);
            const char *t_dirname = (const char*)sqlite3_column_text(res, 2);
            const char *t_maintitle = (const char*)sqlite3_column_text(res, 3);
            const char *t_subtitle = (const char*)sqlite3_column_text(res, 4);
            const char *t_mtime = (const char*)sqlite3_column_text(res, 5);

            // First row A.K.A first Game
            if (!curr_g_id[0]) {
                g = g_head = malloc(sizeof(game));
                memset(g, 0, sizeof(game));
                if (t_id) strncpy(g->id, t_id, sizeof(g->id));
                if (t_sd_id) strncpy(g->sd_id, t_sd_id, sizeof(g->sd_id));
                update_game_title(g, (char*) t_id);
                sd = g->sd_list = malloc(sizeof(savedata));
                memset(sd, 0, sizeof(savedata));
                strncpy(curr_g_id, t_id, sizeof(curr_g_id));
            } else if (strcmp(curr_g_id, t_id)) {           // Change of Game
                g->next = malloc(sizeof(game));
                g = g->next;
                memset(g, 0, sizeof(game));
                if (t_id) strncpy(g->id, t_id, sizeof(g->id));
                if (t_sd_id) strncpy(g->sd_id, t_sd_id, sizeof(g->sd_id));
                update_game_title(g, (char*) t_id);
                sd = g->sd_list = malloc(sizeof(savedata));
                memset(sd, 0, sizeof(savedata));
                strncpy(curr_g_id, t_id, sizeof(curr_g_id));
            } else {                                        // New SD entry for same game
                sd->next = malloc(sizeof(savedata));
                sd = sd->next;
                memset(sd, 0, sizeof(savedata));
            }

            if (t_dirname) strncpy(sd->dir_name, t_dirname, sizeof(sd->dir_name));
            if (t_maintitle) strncpy(sd->main_title, t_maintitle, sizeof(sd->main_title));
            sanitize_string(sd->main_title);
            if (t_subtitle) strncpy(sd->sub_title, t_subtitle, sizeof(sd->sub_title));
            sanitize_string(sd->sub_title);
            if (t_mtime) strncpy(sd->last_time, t_mtime, sizeof(sd->last_time));
            printf("|  %-9.9s  | %-20.20s | %-30.30s | %-20.20s | %-40.40s | %s\n",
                g->id, g->name, sd->dir_name, sd->main_title, sd->sub_title, sd->last_time);
        }
        printf("---------------------------------------------------------------------------------------------------------------------\n");
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return g_head;
}

void update_list_user_games_and_savedata(void) {
    get_users(); // Update list

    int i = 0;

    while (user_list[i].name[0]!=0) {
        printf("\n\nList of SaveData entries for user: %08x\n\n", user_list[i].user_id);
        user_list[i].game_list = get_user_games(i);
        i++;
    }
}

savedata* get_game_last_savedata(game* game) {
    savedata* sd;
    
    if (game->sd_list == NULL)
        return NULL;

    sd = game->sd_list;

    while (sd->next != NULL) {
        sd = sd->next;
    }
    return sd;
}

game* get_user_last_game(user* user) {
    game* game;
    
    if (user->game_list == NULL)
        return NULL;

    game = user->game_list;

    while (game->next != NULL) {
        game = game->next;
    }
    return game;
}

void add_savedata_to_game(game* game, savedata* savedata) {
    get_game_last_savedata(game)->next = savedata;
}

void add_game_to_user(user* user, game* game) {
    get_user_last_game(user)->next = game;
}

void free_savedata_list(game* game) {
    savedata* head;
    savedata* tmp;

    if (game->sd_list == NULL)
        return;

    head = game->sd_list;

    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }

    return;
}

void free_game_list(user* user) {
    game* head;
    game* tmp;

    if (user->game_list == NULL)
        return;

    head = user->game_list;

    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }

    return;
}