#include "tracker_logic.h"

#include "logger.h"
#include "hashmap.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 100

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL



static pthread_mutex_t mutex;


static hashmap_t* user_map;
static hashmap_t* torrent_map;

static uint64_t hash_key(const char* key);
static inline int compare(const char* key, const char* key2);

void tracker_logic_init() {
    
    int r;
    if ((r = pthread_mutex_init(&mutex, NULL)) != 0) {
        LOG_FATAL("pthread_mutex_init(): %d", r);
        return;
    }

    user_map = hashmap_init(compare, hash_key);
    if (user_map == NULL) {
        LOG_FATAL("tracker_logic_init(): failed to initialize user_map");
        return;
    }

    torrent_map = hashmap_init(compare, hash_key);
    if (torrent_map == NULL) {
        LOG_FATAL("tracker_logic_init(): failed to initialize user_map");
        return;
    }
}

void tracker_add_user(const char* unique_id) {
    pthread_mutex_lock(&mutex);

    userinfo_t* new_user = (userinfo_t*)malloc(sizeof(userinfo_t));
    new_user->downloads = 69;
    new_user->uploads = 69;

    hashmap_insert(user_map, unique_id, new_user);

    pthread_mutex_unlock(&mutex);

}



void tracker_remove_user(const char* unique_id) {
    pthread_mutex_lock(&mutex);

    hashmap_remove(user_map, unique_id);

    pthread_mutex_unlock(&mutex);

}


void tracker_remove_torrent(const char* info_hash) {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);

}



userinfo_t* tracker_get_user(const char* unique_id) {
    pthread_mutex_lock(&mutex);

    userinfo_t* user = hashmap_get(user_map, unique_id);

    pthread_mutex_unlock(&mutex);
    return user;
}

torrentfile_t* tracket_get_torrent(const char* info_hash) {
    
}

static uint64_t hash_key(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash % TABLE_SIZE;
}

static inline int compare(const char* key, const char* key2) {
    return strncmp(key, key2, 20);
}


