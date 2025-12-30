#include "hashmap.h"

#include <stdlib.h>
#include <string.h>

#include "logger.h"


#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

static uint64_t hash_key(const char* key);
static inline int compare(const char* key, const char* key2);


hashmap_t* hashmap_init(StorageType type, mem_pool_t* pool) {

    hashmap_t* hashmap = (hashmap_t*)malloc(sizeof(hashmap_t));
    if (!hashmap)
        return NULL;

    hashmap->storageType = type;
    hashmap->pool = pool;

    return hashmap;
}


void hashmap_insert(hashmap_t* hashmap, const char* key, void* value) {

    if (!hashmap || !key) {
        LOG_ERROR("hashmap_insert(): hashmap || key == NULL");
        return;
    }

    U64 hashValue = hash_key(key);

    node_t* current_node = &hashmap->nodes[hashValue % TABLE_SIZE];
    /*
    if (hashmap->nodes[index] != NULL) {

        node_t* current_node = hashmap->nodes[index];
        while(current_node != NULL) {
            if(hashmap->cmp(current_node->key, key) == 0) {
                LOG_DEBUG("hashmap_insert(): node %s already exists. Assigning new value.");
                current_node->value = value;
                return;
            }
            current_node = current_node->next_node;
        }

    }
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    if (!new_node) {
        LOG_ERROR("hashmap_insert(): new_node == NULL");
        return;
    }
    */

    mem_node_t* node = mem_pool_just_alloc_node(hashmap->pool, hashValue, hashmap->storageType);
    if (node == NULL) {
        LOG_ERROR("hashmap_insert(): failed to allocate new node. Probably ran out of space");
        return;
    }

    current_node->root_node = node_avl_add(current_node->root_node, node);
    strncpy(node->userinfo.peer_id, key, 20);
    
}

void hashmap_remove(hashmap_t* hashmap, const char* key) {

    if (!hashmap || !key) {
        LOG_ERROR("hashmap_remove(): hashmap || key == NULL");
        return;
    }


    U64 hashValue = hash_key(key);

    node_t* current_node = &hashmap->nodes[hashValue % TABLE_SIZE];
    if(current_node == NULL)
        return;

    mem_node_t* memNode = node_avl_find(current_node->root_node, hashValue);
    if (memNode == NULL) {
        LOG_DEBUG("hashmap_remove(): nothing to remove");
        return;
    }

    current_node->root_node = node_avl_remove(current_node->root_node, memNode);

    mem_pool_free_node(hashmap->pool, memNode);

    /*
    if (compare(memNode->key, key) == 0) {
        hashmap->nodes[index] = current_node->next_node;
        free(current_node->value);
        free(current_node);
        return;
    }

    node_t* previous_node = NULL;
    while(current_node != NULL) {

        if (compare(current_node->key, key) == 0 && previous_node != NULL) {
            previous_node->next_node = current_node->next_node;
            free(current_node->value);
            free(current_node);

            break;
        }

        previous_node = current_node;
        current_node = current_node->next_node;
    }
    */
}


void* hashmap_get(hashmap_t* hashmap, const char* key) {

    if (!hashmap || !key) {
        LOG_ERROR("hashmap_get(): hashmap || key == NULL");
        return NULL;
    }

    U64 hashValue = hash_key(key);

    node_t* current_node = &hashmap->nodes[hashValue % TABLE_SIZE];

    mem_node_t* memNode = node_avl_find(current_node->root_node, hashValue);

    if (memNode == NULL)
        return NULL;

    if (hashmap->storageType == TORRENTFILE)
        return &memNode->torrentfile;

    if (hashmap->storageType == USERINFO)
        return &memNode->userinfo;

    return NULL;
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


