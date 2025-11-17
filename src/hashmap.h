#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include "common.h"
#include "mem_pool.h"

#define TABLE_SIZE 100



typedef struct node_t {
    union {
        torrentfile_t torrentfile;
        userinfo_t userinfo;
    };
    mem_node_t* root_node;
} node_t;

typedef struct hashmap_t {
    StorageType storageType;
    node_t nodes[TABLE_SIZE];
    mem_pool_t* pool;

} hashmap_t;



hashmap_t* hashmap_init(StorageType type, mem_pool_t* pool);

void hashmap_insert(hashmap_t* hashmap, const char* key, void* value);
void hashmap_remove(hashmap_t* hashmap, const char* key);


void* hashmap_get(hashmap_t* hashmap, const char* key);


#endif