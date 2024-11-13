#include <firmwares/flip_store_firmwares.h>

Firmware *firmwares = NULL;
bool sent_firmware_request = false;
bool sent_firmware_request_2 = false;
bool sent_firmware_request_3 = false;
//
bool firmware_request_success = false;
bool firmware_request_success_2 = false;
bool firmware_request_success_3 = false;
//
bool firmware_download_success = false;
bool firmware_download_success_2 = false;
bool firmware_download_success_3 = false;

Firmware *firmware_alloc()
{
    Firmware *fw = (Firmware *)malloc(FIRMWARE_COUNT * sizeof(Firmware));
    if (!fw)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for Firmware");
        return NULL;
    }
    for (int i = 0; i < FIRMWARE_COUNT; i++)
    {
        if (fw[i].name == NULL)
        {
            fw[i].name = (char *)malloc(16);
            if (!fw[i].name)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for Firmware name");
                return NULL;
            }
        }
        for (int j = 0; j < FIRMWARE_LINKS; j++)
        {
            if (fw[i].links[j] == NULL)
            {
                fw[i].links[j] = (char *)malloc(256);
                if (!fw[i].links[j])
                {
                    FURI_LOG_E(TAG, "Failed to allocate memory for Firmware links");
                    return NULL;
                }
            }
        }
    }

    // Black Magic
    fw[0].name = "Black Magic";
    fw[0].links[0] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/bootloader.bin";
    fw[0].links[1] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/partition-table.bin";
    fw[0].links[2] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/BM/blackmagic.bin";

    // FlipperHTTP
    fw[1].name = "FlipperHTTP";
    fw[1].links[0] = "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/flipper_http_bootloader.bin";
    fw[1].links[1] = "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/flipper_http_firmware_a.bin";
    fw[1].links[2] = "https://raw.githubusercontent.com/jblanked/FlipperHTTP/main/flipper_http_partitions.bin";

    // Marauder
    fw[2].name = "Marauder";
    fw[2].links[0] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.bootloader.bin";
    fw[2].links[1] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.partitions.bin";
    fw[2].links[2] = "https://raw.githubusercontent.com/FZEEFlasher/fzeeflasher.github.io/main/resources/CURRENT/esp32_marauder_v1_0_0_20240626_flipper.bin";

    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/STATIC/BM/bootloader.bin
    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/STATIC/BM/partition-table.bin
    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/STATIC/BM/blackmagic.bin

    // https://api.github.com/repos/jblanked/FlipperHTTP/contents/flipper_http_bootloader.bin
    // https://api.github.com/repos/jblanked/FlipperHTTP/contents/flipper_http_firmware_a.bin
    // https://api.github.com/repos/jblanked/FlipperHTTP/contents/flipper_http_partitions.bin

    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.bootloader.bin
    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/STATIC/M/FLIPDEV/esp32_marauder.ino.partitions.bin
    // https://api.github.com/repos/FZEEFlasher/fzeeflasher.github.io/contents/resources/CURRENT/esp32_marauder_v1_0_0_20240626_flipper.bin
    return fw;
}
void firmware_free()
{
    if (firmwares)
    {
        free(firmwares);
    }
}

bool flip_store_get_firmware_file(char *link, char *name, char *filename)
{
    if (fhttp.state == INACTIVE)
    {
        return false;
    }
    Storage *storage = furi_record_open(RECORD_STORAGE);
    char directory_path[64];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher");
    storage_common_mkdir(storage, directory_path);
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher/%s", firmwares[selected_firmware_index].name);
    storage_common_mkdir(storage, directory_path);
    snprintf(fhttp.file_path, sizeof(fhttp.file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/esp_flasher/%s/%s", name, filename);
    fhttp.save_received_data = false;
    fhttp.is_bytes_request = true;
    char *headers = jsmn("Content-Type", "application/octet-stream");
    bool sent_request = flipper_http_get_request_bytes(link, headers);
    free(headers);
    if (sent_request)
    {
        fhttp.state = RECEIVING;
        return true;
    }
    fhttp.state = ISSUE;
    return false;
}
