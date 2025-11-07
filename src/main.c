#include <stdio.h>
#include "logger.h"

#include "udp_server.h"
#include "tracker_logic.h"
#include "http/http_server.h"

#include <stdlib.h>
#include <uv.h>


int main() {

    
    logger_initConsoleLogger(NULL);
    //logger_initFileLogger("logs/log.txt", 1024 * 1024, 5);

    logger_setLevel(LogLevel_DEBUG);
    
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