#include "mem_pool.h"

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

static inline I32 height(mem_node_t* node);
static inline I32 height2(mem_pool_t* pool, U32 index);
static inline I32 balance_factor(mem_pool_t* pool, mem_node_t* node);

static I32 node_rotate_left(mem_pool_t* pool, I32 i);
static I32 node_rotate_right(mem_pool_t* pool, I32 i);

static inline I32 max(I32 a, I32 b);

static inline mem_node_t* find_min_node(mem_pool_t* pool, mem_node_t* node);
static inline mem_node_t* get(mem_pool_t* pool, I32 i) {
    return (i >= pool->pool_capacity || i < 0) ? NULL : &pool->pool[i];
}

static void pool_reallocate(mem_pool_t* pool, size_t newSize);


void mem_pool_init(mem_pool_t* pool, size_t poolSize) {
    pool->pool = malloc(poolSize * sizeof(mem_node_t));
    pool->free_stack = malloc(poolSize * sizeof(U32));

    for (size_t i = 0; i < poolSize; i++) {
        pool->free_stack[i] = i;
    }
    pool->top = poolSize;

    pool->node_size = sizeof(mem_node_t);
    pool->pool_capacity = poolSize;
    pool->pool_size = 0;


    pool->root_index = -1;
}

static void pool_reallocate(mem_pool_t* pool, size_t newSize) {
    
    mem_node_t* newP = malloc(newSize * sizeof(mem_node_t));
    
    if (newP == NULL) {
        LOG_ERROR("mem_pool_reallocate(): failed to reallocate pool.");
        return;
    }

    U32* newStack = malloc(newSize * sizeof(U32));

    if (newStack == NULL) {
        free(newP);
        LOG_ERROR("mem_pool_reallocate(): failed to reallocate free stack.");
        return;
    }

    memcpy(newP, pool->pool, pool->pool_capacity * pool->node_size);
    memcpy(newStack, pool->free_stack, pool->pool_capacity * sizeof(U32));

    free(pool->pool);
    free(pool->free_stack);

    pool->pool = newP;
    pool->free_stack = newStack;

    for (size_t i = 0; i < newSize - pool->pool_capacity; i++) {
        pool->free_stack[i] = i + newSize - pool->pool_capacity;
    }

    pool->top = newSize - pool->pool_capacity;
    pool->pool_capacity = newSize;
}

void mem_pool_add_node(mem_pool_t* pool, mem_node_t* node) {

    //if (pool->root_index == NULL) {
    //    pool->root_index = node;
    //    return;
    //}

    node_avl_add(pool, pool->root_index, node);
}

I32 mem_pool_just_alloc_node(mem_pool_t* pool, U32 key, StorageType type) {

    if (pool->top == 0) {
        LOG_DEBUG("mem_pool is full");
        pool_reallocate(pool, pool->pool_capacity * 2);
    }
    
    U32 index = pool->free_stack[--pool->top];
    if (pool->root_index < 0)
        pool->root_index = index;

    mem_node_t* node = (mem_node_t*)((char*)pool->pool + index * pool->node_size);
    node->key = key;
    node->height = 1;
    node->leftindex = -1;
    node->rightindex = -1;
    node->type = type;

    return index;
}

I32 mem_pool_alloc_node(mem_pool_t* pool, U32 key, StorageType type) {

    U32 node = mem_pool_just_alloc_node(pool, key, type);
    if (node < 0)
        return -1;

    pool->root_index = node_avl_add(pool, pool->root_index, get(pool, node));

    return node;
}

mem_node_t* mem_pool_find_node(mem_pool_t* pool, U32 key) {
    return node_avl_find(pool, key);
}


void mem_pool_free_node(mem_pool_t* pool, mem_node_t* node) {

    //refresh tree
    node_avl_remove(pool, pool->root_index, node);

    U32 id = ((char*)node - (char*)pool->pool) / pool->node_size;
    
    if (pool->top == pool->pool_capacity) {
        LOG_DEBUG("mem_pool_free_node(): nothing to remove");
        return;
    }

    pool->free_stack[pool->top++] = id;

}

mem_node_t* node_avl_find(mem_pool_t* pool, U32 key) {

    mem_node_t* temp = get(pool, pool->root_index);

    while (temp != NULL) {
        if (temp->key == key)
            return temp;
        else if (temp->key < key)
            temp = get(pool, temp->rightindex);
        else
            temp = get(pool, temp->leftindex);

    }
    return NULL;
}

I32 node_avl_add(mem_pool_t* pool, I32 root_index, mem_node_t* node) {

    mem_node_t* root = get(pool, root_index);

    if (root == NULL) 
        return pool->free_stack[pool->top];

    if (root->key > node->key) {
        root->leftindex = node_avl_add(pool, root->leftindex, node);
    }
    else if (root->key < node->key) {
        root->rightindex = node_avl_add(pool, root->rightindex, node);
    }
    else {
        return root_index;
    }

    mem_node_t* leftNode = get(pool, root->leftindex);
    mem_node_t* rightNode = get(pool, root->rightindex);

    
    root->height = 1 + max(height(leftNode), height(rightNode));

    I32 balanceFactor = balance_factor(pool, root);

    if (balanceFactor > 1 && node->key < leftNode->key) {
        return node_rotate_right(pool, root_index);
    }
    
    if (balanceFactor < -1 && node->key > rightNode->key) {
        return node_rotate_left(pool, root_index);
    }

    if (balanceFactor > 1 && node->key > leftNode->key) {
        root->leftindex = node_rotate_left(pool, root->leftindex);
        return node_rotate_right(pool, root_index);
    }

    if (balanceFactor < -1 && node->key < rightNode->key) {
        root->rightindex = node_rotate_right(pool, root->rightindex);
        return node_rotate_left(pool, root_index);
    }

    return root_index;

}


static inline mem_node_t* find_min_node(mem_pool_t* pool, mem_node_t* node) {
    mem_node_t* current = node;
    while (current->leftindex > 0)
        current = get(pool, current->leftindex);
    return current;
}

I32 node_avl_remove(mem_pool_t* pool, I32 root_index, mem_node_t* node) {

    mem_node_t* root = get(pool, root_index);

    if (root == NULL)
        return -1;

    mem_node_t* leftNode = get(pool, root->leftindex);
    mem_node_t* rightNode = get(pool, root->rightindex);

    if (node->key < root->key) {
        root->leftindex = node_avl_remove(pool, root->leftindex, node);
    }
    else if (node->key > root->key) {
        root->rightindex = node_avl_remove(pool, root->rightindex, node);
    }
    else {
        if (root->leftindex < 0) {
            return root->rightindex;
        }
        else if (root->rightindex < 0) {
            return root->leftindex;
        }
        
        mem_node_t* temp = find_min_node(pool, rightNode);
        root->key = temp->key;

        root->rightindex = node_avl_remove(pool, root->rightindex, temp);
    }

       root->height = 1 + max(height(leftNode), height(rightNode));

    I32 balanceFactor = balance_factor(pool, root);

    if (balanceFactor > 1 && balance_factor(pool, leftNode) >= 0) {
        return node_rotate_right(pool, root_index);
    }
    
    if (balanceFactor < -1 && balance_factor(pool, rightNode) <= 0) {
        return node_rotate_left(pool, root_index);
    }

    if (balanceFactor > 1 && balance_factor(pool, leftNode) < 0) {
        root->leftindex = node_rotate_left(pool, root->leftindex);
        return node_rotate_right(pool, root_index);
    }

    if (balanceFactor < -1 && balance_factor(pool, rightNode) > 0) {
        root->rightindex = node_rotate_right(pool, root->rightindex);
        return node_rotate_left(pool, root_index);
    }

    return root_index;
}

static inline I32 height(mem_node_t* node) {
    if (!node)
        return 0;
    return node->height;
}

static inline I32 height2(mem_pool_t* pool, U32 index) {
    mem_node_t* node = get(pool, index);
    if (!node)
        return 0;
    return node->height;
}

static inline I32 max(I32 a, I32 b) {
    return a > b ? a : b;
}

static inline I32 balance_factor(mem_pool_t* pool, mem_node_t* node) {
    return height2(pool, node->leftindex) - height2(pool, node->rightindex);
}

static I32 node_rotate_left(mem_pool_t* pool, I32 i) 
{
    mem_node_t* x = get(pool, i);
    I32 ret = x->rightindex;

    mem_node_t* y = get(pool, x->rightindex);


    x->rightindex = y->leftindex;
    y->leftindex = i;

    x->height = 1 + max(height2(pool, x->leftindex), height2(pool, x->rightindex));
    y->height = 1 + max(height2(pool, y->leftindex), height2(pool, y->rightindex));

    return ret;
}

static I32 node_rotate_right(mem_pool_t* pool, I32 i) {
    
    mem_node_t* head = get(pool, i);

    mem_node_t* newHead = get(pool, head->leftindex);
    I32 ret = head->leftindex;
    I32 t3 = newHead->rightindex;

    newHead->rightindex = i;
    head->leftindex = t3;

    head->height = 1 + max(height2(pool, head->leftindex), height2(pool, head->rightindex));
    newHead->height = 1 + max(height2(pool, newHead->leftindex), height2(pool, newHead->rightindex));

    return ret;
}
