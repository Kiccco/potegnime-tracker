#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "common.h"

typedef struct user_node_t user_node_t;
typedef struct torrent_node_t torrent_node_t;
typedef struct mem_node_t mem_node_t;


typedef struct mem_node_t {
    U32 key;
    I32 height;
    //mem_node_t* left;
    //mem_node_t* right;
    I32 leftindex;
    I32 rightindex;
    StorageType type;
    union {
        userinfo_t userinfo;
        torrentfile_t torrentfile;
        account_info_t accountinfo;
    };
    
} mem_node_t;

typedef struct mem_pool_t {
    mem_node_t* pool;
    size_t pool_size;
    size_t pool_capacity;
    size_t node_size;
    U32* free_stack;
    U32 top;
    I32 root_index;

} mem_pool_t;


void mem_pool_init(mem_pool_t* pool, size_t poolSize);

void mem_pool_add_node(mem_pool_t* pool, mem_node_t* node);
mem_node_t* mem_pool_find_node(mem_pool_t* pool, U32 key);

I32 mem_pool_just_alloc_node(mem_pool_t* pool, U32 key, StorageType type);
I32 mem_pool_alloc_node(mem_pool_t* pool, U32 key, StorageType type);
void mem_pool_free_node(mem_pool_t* pool, mem_node_t* node);


I32 node_avl_add(mem_pool_t* pool, I32 root_index, mem_node_t* node);
I32 node_avl_remove(mem_pool_t* pool, I32 root_index, mem_node_t* node);
mem_node_t* node_avl_find(mem_pool_t* pool, U32 key);

#endif