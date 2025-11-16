#include "http_server.h"

#include "../logger.h"
#include "../common.h"
#include <immintrin.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#define HTTP_PORT 8080
#define MAX_HEADERS 5


typedef struct http_headers_t {
    const char* key;
    U32 key_len;

    const char* value;
    U32 value_len;

} http_headers_t;

typedef http_headers_t http_param_t;


static uv_tcp_t server;

static void on_new_connection(uv_stream_t *server, int status);
static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
	*buf = uv_buf_init(malloc(size), size);
}

static void parse_request(const char* buf, const char* buf_end, char* method);
static const char* parse_token(const char* buf, const char* buf_end, char search_char, char** token, U32* token_len);
static void parse_uri(const char* buf, const char* buf_end);
static void decode_urlencoded_param(const char* buf, const char* buf_end, char* dest, U32 max_len);


void http_server_init(uv_loop_t* loop) {

    struct sockaddr_in addr;

    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", 8080, &addr);

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &server, 1024, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return;
    }

    LOG_INFO("init web server on port: %u", HTTP_PORT);

}   

static void on_new_connection(uv_stream_t *server, int status) {

    LOG_DEBUG("incoming new connection...");

    if (status < 0) {
        LOG_ERROR("New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    
    uv_tcp_init(server->loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_cb, on_read);
    }
}

static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {

    U8* ptr = buf->base;
    if (nread < 0) {
        LOG_DEBUG("disconnected read error: %s", uv_err_name(nread));
        uv_close((uv_handle_t*)stream, NULL);
        free(buf->base);
        return;
    }
    
    char method[10];
    parse_request(buf->base, buf->base + buf->len, method);

    const char* res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\ntest";
    

    uv_write_t req;
    uv_write(&req, stream, (const uv_buf_t[]) {
        { .base = res, .len = strlen(res)}
    }, 1, NULL);

    uv_close((uv_handle_t*)stream, NULL);
    free(buf->base);

}

static void parse_request(const char* buf, const char* buf_end, char* method) {

    char* method_test = NULL;
    U32 method_len = 0;
    //method
    buf = parse_token(buf, buf_end, ' ', &method_test, &method_len);
    if (strncmp(method_test, "GET", method_len) == 0) {
        LOG_DEBUG("Got GET method");
    }

    //uri
    char* uri = NULL;
    U32 uri_len = 0;

    buf = parse_token(buf, buf_end, ' ', &uri, &uri_len);
    LOG_DEBUG("URI: %.*s", uri_len, uri);


    //http version
    char* version = NULL;
    U32 version_len = 0;
    buf = parse_token(buf, buf_end, '\n', &version, &version_len);
    LOG_DEBUG("HTTP VERSION: %.*s", version_len, version);

    http_headers_t headers[MAX_HEADERS] = {};
    U32 i = 0;

    LOG_DEBUG("parsing headers...");
    while (buf != buf_end && i < MAX_HEADERS) {
        char* key = NULL;
        U32 key_len = 0;

        buf = parse_token(buf, buf_end, ':', &key, &key_len);
        if (buf == NULL) {
            LOG_DEBUG("reached to the end when parsing headers (key)");
            break;
        }

        ++buf;

        char* value = NULL;
        U32 value_len = 0;

        buf = parse_token(buf, buf_end, '\r', &value, &value_len);
        if (buf == NULL) {
            LOG_DEBUG("reached to the end when parsing headers (value)");
            break;
        }

        if (*++buf == '\n') {
            buf++;
        }

        LOG_DEBUG("header: %.*s value: %.*s", key_len, key, value_len, value);
        
        headers[i].key = key;
        headers[i].key_len = key_len;
        headers[i].value = value;
        headers[i].value_len = value_len;

        i++;
    }

    parse_uri(uri, uri + uri_len);


}

static void parse_uri(const char* buf, const char* buf_end) {

    char* path = NULL;
    U32 path_len = 0;

    const char* buf_start = buf;

    buf = parse_token(buf, buf_end, '?', &path, &path_len);
    if (buf == NULL) {
        //kva pol ce je sam npr. /announce
        return;
    }

    while (buf <= buf_end) {

        char* query = NULL;
        U32 query_len = 0;

        char* value = NULL;
        U32 value_len = 0;
        
        buf = parse_token(buf, buf_end, '=', &query, &query_len);
        if (buf == NULL) {
            LOG_DEBUG("buf == NULL when parsing query name.");
            break;
        }

        buf = parse_token(buf, buf_end, '&', &value, &value_len);
        if (buf == NULL) {
            LOG_DEBUG("buf == NULL when parsing query value.");
            break;
        }

        if (strncmp(query, "info_hash", query_len) == 0) {
            unsigned char info_hash[20] = {};
            decode_urlencoded_param(value, value + value_len, info_hash, 20);
            
        }
        
        
    }

    if (strncmp(path, "/announce", path_len) == 0) {

    }
    else if (strncmp(path, "/scrape", path_len) == 0) {
        LOG_DEBUG("Scrape not implemented");
    }
}


static unsigned char fromhex(unsigned char x) {
  x-='0'; if( x<=9) return x;
  x&=~0x20; x-='A'-'0';
  if( x<6 ) return x+10;
  return 0xff;
}

static void decode_urlencoded_param(const char* buf, const char* buf_end, char* dest, U32 max_len) {

    U32 i = 0;
    U8 hi = 0;
    U8 lo = 0;
    U8 res = 0;
    while (buf <= buf_end && i < max_len) {

        if (*buf++ == '%') {
            hi = fromhex(*buf++);
            lo = fromhex(*buf++);
            *dest++ = (hi << 4) | lo;
        }
        else {
            *dest++ = *buf;
        }

        i++;
    }
}

static const char* parse_token(const char* buf, const char* buf_end, char search_char, char** token, U32* token_len) {

    const char* buf_start = buf;

    char* end = strchr(buf, search_char);
    if (end == NULL)
        return NULL;

    while (1) {

        if (buf == buf_end) {
            break;
        }

        if (*buf == search_char) {
            break;
        }
        ++buf;
    }

    *token = buf_start;
    *token_len = buf - buf_start;

    if (buf + 1 != buf_end)
        buf++;
    
    return buf;
}

