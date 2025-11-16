#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "common.h"
#include "tracker_logic.h"

typedef struct user_node_t user_node_t;
typedef struct torrent_node_t torrent_node_t;
typedef struct mem_node_t mem_node_t;

typedef struct user_node_t {
    U32 key;
    I32 height;
    user_node_t* left;
    user_node_t* right;
    userinfo_t userinfo;
} user_node_t;

typedef struct torrent_node_t {
    U32 key;
    I32 height;
    torrent_node_t* left;
    torrent_node_t* right;
    torrentfile_t torrentinfo;
} torrent_node_t;

typedef struct mem_node_t {
    U32 key;
    I32 height;
    mem_node_t* left;
    mem_node_t* right;
} mem_node_t;

typedef struct mem_pool_t {
    mem_node_t* pool;
    size_t pool_size;
    size_t pool_capacity;
    size_t node_size;
    U32 next_free_index;
    U32* free_stack;
    U32 top;
    mem_node_t* root_node;

} mem_pool_t;


void mem_pool_init(mem_pool_t* pool, size_t poolSize, size_t dataSize);

void mem_pool_add_node(mem_pool_t* pool, mem_node_t* node);
mem_node_t* mem_pool_find_node(mem_pool_t* pool, U32 key);


mem_node_t* mem_pool_alloc_node(mem_pool_t* pool, U32 key);
void mem_pool_free_node(mem_pool_t* pool, mem_node_t* node);

#endif