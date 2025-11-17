#include "mem_pool.h"

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"


static inline I32 height(mem_node_t* node);
static inline I32 balance_factor(mem_node_t* node);

static mem_node_t* node_rotate_left(mem_node_t* node);
static mem_node_t* node_rotate_right(mem_node_t* node);

static inline I32 max(I32 a, I32 b);

static mem_node_t* find_min_node(mem_node_t* node);


void mem_pool_init(mem_pool_t* pool, size_t poolSize) {
    pool->pool = malloc(poolSize * sizeof(mem_node_t));
    pool->free_stack = malloc(poolSize * sizeof(U32));

    for (size_t i = 0; i < poolSize; i++) {
        pool->free_stack[i] = i;
    }
    pool->top = poolSize;

    pool->node_size = sizeof(mem_node_t);
    pool->pool_capacity = poolSize;

    pool->next_free_index = 0;
    pool->pool_size = 0;

    pool->root_node = NULL;
}

void mem_pool_add_node(mem_pool_t* pool, mem_node_t* node) {

    if (pool->root_node == NULL) {
        pool->root_node = node;
        return;
    }

    node_avl_add(pool->root_node, node);
}

mem_node_t* mem_pool_just_alloc_node(mem_pool_t* pool, U32 key, StorageType type) {

    if (pool->top == 0) {
        LOG_DEBUG("mem_pool is full");
        return NULL;
    }
    
    U32 index = pool->free_stack[--pool->top];

    mem_node_t* node = (mem_node_t*)((char*)pool->pool + index * pool->node_size);
    node->key = key;
    node->height = 1;
    node->left = 0;
    node->right = 0;
    node->type = type;

    return node;
}

mem_node_t* mem_pool_alloc_node(mem_pool_t* pool, U32 key, StorageType type) {

    mem_node_t* node = mem_pool_just_alloc_node(pool, key, type);
    if (node == NULL)
        return NULL;

    pool->root_node = node_avl_add(pool->root_node, node);

    return node;
}

mem_node_t* mem_pool_find_node(mem_pool_t* pool, U32 key) {
    return node_avl_find(pool->root_node, key);
}


void mem_pool_free_node(mem_pool_t* pool, mem_node_t* node) {

    //refresh tree
    node_avl_remove(pool->root_node, node);

    U32 id = ((char*)node - (char*)pool->pool) / pool->node_size;
    
    if (pool->top == pool->pool_capacity) {
        LOG_DEBUG("mem_pool_free_node(): nothing to remove");
        return;
    }

    pool->free_stack[pool->top++] = id;

}

mem_node_t* node_avl_find(mem_node_t* root, U32 key) {

    mem_node_t* temp = root;

    while (temp != NULL) {
        if (temp->key == key)
            return temp;
        else if (temp->key < key)
            temp = temp->right;
        else
            temp = temp->left;

    }
    return NULL;
}

mem_node_t* node_avl_add(mem_node_t* root, mem_node_t* node) {

    if (root == NULL) 
        return node;
    
    if (root->key > node->key) {
        root->left = node_avl_add(root->left, node);
    }
    else if (root->key < node->key) {
        root->right = node_avl_add(root->right, node);
    }
    else {
        return root;
    }
    
    root->height = 1 + max(height(root->left), height(root->right));

    I32 balanceFactor = balance_factor(root);

    if (balanceFactor > 1 && node->key < root->left->key) {
        return node_rotate_right(root);
    }
    
    if (balanceFactor < -1 && node->key > root->right->key) {
        return node_rotate_left(root);
    }

    if (balanceFactor > 1 && node->key > root->left->key) {
        root->left = node_rotate_left(root->left);
        return node_rotate_right(root);
    }

    if (balanceFactor < -1 && node->key < root->right->key) {
        root->right = node_rotate_right(root->right);
        return node_rotate_left(root);
    }

    return root;

}


static mem_node_t* find_min_node(mem_node_t* node) {
    mem_node_t* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

mem_node_t* node_avl_remove(mem_node_t* root, mem_node_t* node) {

    if (root == NULL)
        return NULL;

    if (node->key < root->key) {
        root->left = node_avl_remove(root->left, node);
    }
    else if (node->key > root->key) {
        root->right = node_avl_remove(root->right, node);
    }
    else {
        if (root->left == NULL) {
            return root->right;
        }
        else if (root->right == NULL) {
            return root->left;
        }
        
        mem_node_t* temp = find_min_node(root->right);
        root->key = temp->key;

        root->right = node_avl_remove(root->right, temp);
    }

       root->height = 1 + max(height(root->left), height(root->right));

    I32 balanceFactor = balance_factor(root);

    if (balanceFactor > 1 && balance_factor(root->left) >= 0) {
        return node_rotate_right(root);
    }
    
    if (balanceFactor < -1 && balance_factor(root->right) <= 0) {
        return node_rotate_left(root);
    }

    if (balanceFactor > 1 && balance_factor(root->left) < 0) {
        root->left = node_rotate_left(root->left);
        return node_rotate_right(root);
    }

    if (balanceFactor < -1 && balance_factor(root->right) > 0) {
        root->right = node_rotate_right(root->right);
        return node_rotate_left(root);
    }

    return root;
}

static inline I32 height(mem_node_t* node) {
    if (!node)
        return 0;
    return node->height;
}

static inline I32 max(I32 a, I32 b) {
    return a > b ? a : b;
}

static inline I32 balance_factor(mem_node_t* node) {
    return height(node->left) - height(node->right);
}

static mem_node_t* node_rotate_left(mem_node_t* x) 
{
    mem_node_t* y = x->right;
    mem_node_t* t3 = y->left;

    x->right = t3;
    y->left = x;

    x->height = 1 + max(height(x->left), height(x->right));
    y->height = 1 + max(height(y->left), height(y->right));

    return y;
}

static mem_node_t* node_rotate_right(mem_node_t* head) {
    mem_node_t* newHead = head->left;
    mem_node_t* t3 = newHead->right;

    newHead->right = head;
    head->left = t3;

    head->height = 1 + max(height(head->left), height(head->right));
    newHead->height = 1 + max(height(newHead->left), height(newHead->right));

    return newHead;
}
