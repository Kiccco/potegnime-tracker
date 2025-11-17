#ifndef COMMON_H
#define COMMON_H

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long U64;

typedef char I8;
typedef short I16;
typedef int I32;
typedef long I64;

typedef float F32;

typedef enum EVENT {
    EVENT_STARTED = 0,
    EVENT_COMPLETED,
    EVENT_STOPPED
} EVENT;

typedef enum StorageType {
    TORRENTFILE,
    USERINFO
} StorageType;


typedef struct account_info_t {
    char auth_key[20];
    U32 totalDownload;
    U32 totalUploads;
} account_info_t;

typedef struct torrentfile_t {
    char info_hash[20];
    U32 seeders;
    U32 lecheers;
    U32 completed;

} torrentfile_t;

typedef struct userinfo_t {
    char peer_id[20];
    U32 address;
    U16 port;
    U32 downloads;
    U32 uploads;
    U32 left;
    EVENT event;
    U32 numwant;
    torrentfile_t* torrent;
    account_info_t* account;
} userinfo_t;


#ifndef _MSC_VER
typedef unsigned long size_t;
#endif
#endif