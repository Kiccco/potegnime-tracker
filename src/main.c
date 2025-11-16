#include <stdio.h>
#include "logger.h"

#include "udp_server.h"

#include "http/http_server.h"

#include <stdlib.h>
#include <uv.h>

#include "mem_pool.h"


int main() {

    
    logger_initConsoleLogger(NULL);
    //logger_initFileLogger("logs/log.txt", 1024 * 1024, 5);

    logger_setLevel(LogLevel_DEBUG);

    mem_pool_t pool;
    mem_pool_init(&pool, 1024, sizeof(user_node_t));

    for (U32 i = 0; i < 1024; i++) {

    }

    user_node_t* node = (user_node_t*)mem_pool_alloc_node(&pool, 3);
    user_node_t* node2 = (user_node_t*)mem_pool_alloc_node(&pool, 5);
    user_node_t* node3 = (user_node_t*)mem_pool_alloc_node(&pool, 6);
    user_node_t* node4 = (user_node_t*)mem_pool_alloc_node(&pool, 10);


    user_node_t* node5 = (user_node_t*)mem_pool_alloc_node(&pool, 15);
    mem_pool_alloc_node(&pool, 2);
    mem_pool_alloc_node(&pool, 1);
    mem_pool_alloc_node(&pool, 30);
    mem_pool_free_node(&pool, (mem_node_t*)node4);
    node4 = NULL;

    user_node_t* node30 = (user_node_t*)mem_pool_find_node(&pool, 30);
    user_node_t* node40 = (user_node_t*)mem_pool_find_node(&pool, 40);


    mem_pool_free_node(&pool, (mem_node_t*)node2);

    mem_pool_free_node(&pool, (mem_node_t*)node3);

    node4 = (user_node_t*)mem_pool_alloc_node(&pool, 2);

    
    uv_loop_t *loop = uv_default_loop();

    tracker_logic_init();
    http_server_init(loop);


    LOG_INFO("Starting event loop.");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);

    //udp_init(6969);

    //udp_deinit();

    return 0;
}