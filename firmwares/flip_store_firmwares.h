#ifndef FLIP_STORE_FIRMWARES_H
#define FLIP_STORE_FIRMWARES_H

#include <flip_store.h>
#include <flip_storage/flip_store_storage.h>
#include <callback/flip_store_callback.h>

typedef struct
{
    char *name;
    char *links[FIRMWARE_LINKS];
} Firmware;

extern Firmware *firmwares;
Firmware *firmware_alloc();
void firmware_free();

// download and waiting process
bool flip_store_get_firmware_file(FlipperHTTP *fhttp, char *link, char *name, char *filename);

extern bool sent_firmware_request;
extern bool sent_firmware_request_2;
extern bool sent_firmware_request_3;
extern bool firmware_request_success;
extern bool firmware_request_success_2;
extern bool firmware_request_success_3;
extern bool firmware_download_success;
extern bool firmware_download_success_2;
extern bool firmware_download_success_3;

#endif // FLIP_STORE_FIRMWARES_H