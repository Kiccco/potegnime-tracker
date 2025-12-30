#include "tracker_logic.h"

#include "logger.h"
#include "hashmap.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 100



static pthread_mutex_t mutex;

static mem_pool_t mem_pool;


static hashmap_t* user_map;
static hashmap_t* torrent_map;

static mem_pool_t user_pool;

void tracker_logic_init() {

    mem_pool_init(&mem_pool, 128);

    mem_pool_init(&user_pool, 128);
    
    int r;
    if ((r = pthread_mutex_init(&mutex, NULL)) != 0) {
        LOG_FATAL("pthread_mutex_init(): %d", r);
        return;
    }

}

void tracker_add_user(const char* unique_id, U32 ip, U16 port, U32 numwant) {
    pthread_mutex_lock(&mutex);

    //userinfo_t* new_user = (userinfo_t*)malloc(sizeof(userinfo_t));
    //new_user->downloads = 69;
    //new_user->uploads = 69;

    //hashmap_insert(user_map, unique_id, new_user);

    pthread_mutex_unlock(&mutex);

}



void tracker_remove_user(const char* unique_id) {
    pthread_mutex_lock(&mutex);

    hashmap_remove(user_map, unique_id);

    pthread_mutex_unlock(&mutex);

}


void tracker_add_torrent(const char* info_hash) {
    pthread_mutex_lock(&mutex);



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



