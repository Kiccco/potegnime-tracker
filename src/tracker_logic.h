#ifndef TRACKER_LOGIC_H
#define TRACKER_LOGIC

#include "common.h"

typedef enum EVENT {
    EVENT_STARTED = 0,
    EVENT_COMPLETED,
    EVENT_STOPPED
} EVENT;

typedef struct account_info_t {
    char auth_key[20];
    U32 totalDownload;
    U32 totalUploads;
} account_info_t;

typedef struct torrentfile_t {
    char info_hash[20];
    U32 seeders;
    U32 lecheers;
    U32 completed;

} torrentfile_t;

typedef struct userinfo_t {
    char peer_id[20];
    U32 address;
    U16 port;
    U32 downloads;
    U32 uploads;
    U32 left;
    EVENT event;
    U32 numwant;
    torrentfile_t* torrent;
    account_info_t* account;
} userinfo_t;




void tracker_logic_init();

void tracker_add_user(const char* unique_id);
void tracker_add_torrent(const char* info_hash);


void tracker_remove_user(const char* unique_id);
void tracker_remove_torrent(const char* info_hash);


userinfo_t* tracker_get_user(const char* unique_id);
torrentfile_t* tracket_get_torrent(const char* info_hash);

#endif
