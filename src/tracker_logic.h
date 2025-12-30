#ifndef TRACKER_LOGIC_H
#define TRACKER_LOGIC

#include "common.h"






void tracker_logic_init();

void tracker_add_user(const char* unique_id, U32 ip, U16 port, U32 numwant);
void tracker_add_torrent(const char* info_hash);


void tracker_remove_user(const char* unique_id);
void tracker_remove_torrent(const char* info_hash);


userinfo_t* tracker_get_user(const char* unique_id);
torrentfile_t* tracket_get_torrent(const char* info_hash);

#endif
