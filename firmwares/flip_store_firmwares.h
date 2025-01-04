#ifndef FLIP_STORE_FIRMWARES_H
#define FLIP_STORE_FIRMWARES_H

#include <flip_store.h>
#include <flip_storage/flip_store_storage.h>
#include <callback/flip_store_callback.h>

typedef struct
{
    char name[16];
    char links[FIRMWARE_LINKS][256];
} Firmware;

typedef struct
{
    char name[16];
    char link[256];
} VGMFirmware;

extern Firmware *firmwares;
extern VGMFirmware *vgm_firmwares;
Firmware *firmware_alloc();
VGMFirmware *vgm_firmware_alloc();
void firmware_free();
void vgm_firmware_free();
extern bool is_esp32_firmware;

// download and waiting process
bool flip_store_get_firmware_file(FlipperHTTP *fhttp, char *link, char *name, char *filename);

#endif // FLIP_STORE_FIRMWARES_H