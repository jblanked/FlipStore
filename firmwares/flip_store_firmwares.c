#include <firmwares/flip_store_firmwares.h>

Firmware *firmwares = NULL;
VGMFirmware *vgm_firmwares = NULL;
bool is_esp32_firmware = true;

Firmware *firmware_alloc()
{
    Firmware *fw = (Firmware *)malloc(FIRMWARE_COUNT * sizeof(Firmware));
    if (!fw)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for Firmware");
        return NULL;
    }

    // Black Magic
    snprintf(fw[0].name, sizeof(fw[0].name), "%s", "Black Magic");
    snprintf(fw[0].links[0], sizeof(fw[0].links[0]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/bootloader.bin");
    snprintf(fw[0].links[1], sizeof(fw[0].links[1]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/partition-table.bin");
    snprintf(fw[0].links[2], sizeof(fw[0].links[2]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/blackmagic.bin");

    // FlipperHTTP
    snprintf(fw[1].name, sizeof(fw[1].name), "%s", "FlipperHTTP");
    snprintf(fw[1].links[0], sizeof(fw[1].links[0]), "%s", "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/WiFi%20Developer%20Board%20(ESP32S2)/flipper_http_bootloader.bin");
    snprintf(fw[1].links[1], sizeof(fw[1].links[1]), "%s", "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/WiFi%20Developer%20Board%20(ESP32S2)/flipper_http_firmware_a.bin");
    snprintf(fw[1].links[2], sizeof(fw[1].links[2]), "%s", "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/WiFi%20Developer%20Board%20(ESP32S2)/flipper_http_partitions.bin");

    // Marauder
    snprintf(fw[2].name, sizeof(fw[2].name), "%s", "Marauder");
    snprintf(fw[2].links[0], sizeof(fw[2].links[0]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.bootloader.bin");
    snprintf(fw[2].links[1], sizeof(fw[2].links[1]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.partitions.bin");
    snprintf(fw[2].links[2], sizeof(fw[2].links[2]), "%s", "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/CURRENT/esp32_marauder_v1_2_0_12192024_flipper.bin");

    return fw;
}

VGMFirmware *vgm_firmware_alloc()
{
    VGMFirmware *fw = (VGMFirmware *)malloc(VGM_FIRMWARE_COUNT * sizeof(VGMFirmware));
    if (!fw)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for VGM Firmware");
        return NULL;
    }

    // FlipperHTTP
    snprintf(fw[0].name, sizeof(fw[0].name), "%s", "FlipperHTTP");
    snprintf(fw[0].link, sizeof(fw[0].link), "%s", "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/Video%20Game%20Module/MicroPython/flipper_http_vgm_micro_python.uf2");

    return fw;
}

void firmware_free()
{
    if (firmwares)
    {
        free(firmwares);
        firmwares = NULL;
    }
}
void vgm_firmware_free()
{
    if (vgm_firmwares)
    {
        free(vgm_firmwares);
        vgm_firmwares = NULL;
    }
}

bool flip_store_get_firmware_file(FlipperHTTP *fhttp, char *link, char *name, char *filename)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    if (fhttp->state == INACTIVE)
    {
        return false;
    }

    Storage *storage = furi_record_open(RECORD_STORAGE);

    char directory_path[64];
    // save in ESP32 flasher directory
    if (is_esp32_firmware)
    {
        snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher");
        storage_common_mkdir(storage, directory_path);
        snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher/%s", firmwares[selected_firmware_index].name);
        storage_common_mkdir(storage, directory_path);
        snprintf(fhttp->file_path, sizeof(fhttp->file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher/%s/%s", name, filename);
    }
    else // install in app_data directory
    {
        snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/vgm");
        storage_common_mkdir(storage, directory_path);
        snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/vgm/%s", name);
        storage_common_mkdir(storage, directory_path);
        snprintf(fhttp->file_path, sizeof(fhttp->file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/vgm/%s/%s", name, filename);
    }
    furi_record_close(RECORD_STORAGE);
    fhttp->save_received_data = false;
    fhttp->is_bytes_request = true;
    return flipper_http_get_request_bytes(fhttp, link, "{\"Content-Type\":\"application/octet-stream\"}");
}
