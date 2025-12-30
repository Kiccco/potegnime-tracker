#include "http_server.h"

#include "../logger.h"
#include "../common.h"
#include <immintrin.h>
#include <string.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#include "../tracker_logic.h"

#define HTTP_PORT 8080
#define MAX_HEADERS 5

#define PEER_ID_LEN 20
#define AUTH_ID_LEN 40
#define INFO_HASH_LEN 20

typedef struct {
    const char* value;
    size_t value_len;
} search_value;

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

static I32 parse_request(uv_stream_t* stream, const char* buf, const char* buf_end, char* method, http_headers_t* headers);
static const char* parse_token(const char* buf, const char* buf_end, char search_char, char** token, size_t* token_len);
static I32 parse_uri(uv_stream_t* stream, const char* buf, const char* buf_end);
static size_t decode_urlencoded_param(const char* buf, const char* buf_end, char* dest, U32 max_len);


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

    http_headers_t headers[MAX_HEADERS] = {};


    I32 code = parse_request(stream, buf->base, buf->base + buf->len, method, headers);

    const char* res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\ntest";
    

    uv_write_t req;

    uv_write(&req, stream, (const uv_buf_t[]) {
        { .base = res, .len = strlen(res)}
    }, 1, NULL);

    uv_close((uv_handle_t*)stream, NULL);
    free(buf->base);

}

static I32 parse_request(uv_stream_t* stream, const char* buf, const char* buf_end, char* method, http_headers_t* headers) {

    char* method_test = NULL;
    size_t method_len = 0;
    //method
    buf = parse_token(buf, buf_end, ' ', &method_test, &method_len);
    if (strncmp(method_test, "GET", method_len) == 0) {
        LOG_DEBUG("Got GET method");
    }

    //uri
    char* uri = NULL;
    size_t uri_len = 0;

    buf = parse_token(buf, buf_end, ' ', &uri, &uri_len);
    LOG_DEBUG("URI: %.*s", uri_len, uri);


    //http version
    char* version = NULL;
    size_t version_len = 0;
    buf = parse_token(buf, buf_end, '\n', &version, &version_len);
    LOG_DEBUG("HTTP VERSION: %.*s", version_len, version);

    U32 i = 0;

    LOG_DEBUG("parsing headers...");
    while (buf != buf_end && i < MAX_HEADERS) {
        char* key = NULL;
        size_t key_len = 0;

        buf = parse_token(buf, buf_end, ':', &key, &key_len);
        if (buf == NULL) {
            LOG_DEBUG("reached to the end when parsing headers (key)");
            break;
        }

        ++buf;

        char* value = NULL;
        size_t value_len = 0;

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

    return parse_uri(stream, uri, uri + uri_len);
}


static const search_value search_values[] = {
    { "auth", 4 }, { "info_hash",  9 }, { "peer_id", 7 }, { "port", 4 }, { "uploaded", 8 }, { "downloaded", 10 }, { "left", 4 }, { "event", 5 }, { "numwant", 7 }
};

static I32 parse_query(const char** buf, const char** buf_end, char** query, size_t* query_len, char** value, size_t* value_len) {
 
        *buf = parse_token(*buf, *buf_end, '=', query, query_len);
        if (*buf == NULL) {
            LOG_DEBUG("buf == NULL when parsing query name.");
            return -3;
        }

        *buf = parse_token(*buf, *buf_end, '&', value, value_len);
        if (*buf == NULL) {
            LOG_DEBUG("buf == NULL when parsing query value.");
            return -3;
        }

        search_value* ptr = search_values;

        size_t s = sizeof(search_values) / sizeof(search_value);

        while (ptr <= &search_values[s - 1]) {

            if (*query_len == ptr->value_len && memcmp(*query, ptr->value, ptr->value_len) == 0) {
                return ptr - &search_values[0];
            }
            ptr++;
        }

    return -1;
}

static I32 parse_uri(uv_stream_t* stream, const char* buf, const char* buf_end) {

    char* path = NULL;
    size_t path_len = 0;

    const char* buf_start = buf;

    buf = parse_token(buf, buf_end, '?', &path, &path_len);
    if (buf == NULL)
        return -20;


    U8 info_hash[INFO_HASH_LEN] = {};
    U8 peer_id[PEER_ID_LEN] = {};
    U8 auth[AUTH_ID_LEN] = {};
    U16 port = 0;

    U32 uploaded = 0;
    U32 downloaded = 0;
    U32 left = 0;

    EVENT event = EVENT_STARTED;

    char* query = NULL;
    size_t query_len = 0;

    char* value = NULL;
    size_t value_len = 0;

    while (buf <= buf_end) {

        I32 res = parse_query(&buf, &buf_end, &query, &query_len, &value, &value_len);
        if (res == -3)
            break;

        switch (res)
        {
            case 0: { //auth
                break;
            }

            case 1: { //info_hash
                if (decode_urlencoded_param(value, value + value_len, info_hash, INFO_HASH_LEN) != INFO_HASH_LEN) {
                    return -21;
                }
                break;
            }
            case 2: { //peer_id
                if (decode_urlencoded_param(value, value + value_len, peer_id, PEER_ID_LEN) != PEER_ID_LEN) {
                    return -21;
                }

                break;
            }
            case 3: { //port
                port = strtoul(value, (char**)(&value + value_len), 10);
                if (port == 0) {
                    return -21;
                }

                break;
            }
            case 4: { //uploaded
                uploaded = strtoul(value, (char**)(&value + value_len), 10);
                break;
            }
            case 5: {
                downloaded = strtoul(value, (char**)(&value + value_len), 10);
                break;
            }
            case 6: { //left

            }
            case 7: { //event
                if (memcmp(value, "started", value_len) == 0)
                    event = EVENT_STARTED;
                else if (memcmp(value, "stopped", value_len) == 0)
                    event = EVENT_STOPPED;
                else if (memcmp(value, "completed", value_len) == 0)
                    event = EVENT_COMPLETED;

                break;
            }
            case -1:
            default:
                break;
        }
    }

    if (strncmp(path, "/announce", path_len) == 0) {
        LOG_DEBUG("info_hash: %x, peer_id: %s, port: %u, downloaded: %u, uploaded: %u, event: %d \n",
                info_hash, peer_id, port, downloaded, uploaded, event);
        
        return 0;
    }
    else if (strncmp(path, "/scrape", path_len) == 0) {
        LOG_DEBUG("Scrape not implemented");
        return -2;
    }

    return -1;
}




static unsigned char fromhex(unsigned char x) {
  x-='0'; if( x<=9) return x;
  x&=~0x20; x-='A'-'0';
  if( x<6 ) return x+10;
  return 0xff;
}

static size_t decode_urlencoded_param(const char* buf, const char* buf_end, char* dest, U32 max_len) {

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

    return i;
}

static const char* parse_token(const char* buf, const char* buf_end, char search_char, char** token, size_t* token_len) {

    char* buf_start = buf;

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

static void http_send_error() {

}
