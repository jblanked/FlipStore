#include <apps/flip_store_apps.h>

FlipStoreAppInfo *flip_catalog = NULL;

uint32_t app_selected_index = 0;
bool flip_store_sent_request = false;
bool flip_store_success = false;
bool flip_store_saved_data = false;
bool flip_store_saved_success = false;
uint32_t flip_store_category_index = 0;

// define the list of categories
char *category_ids[] = {
    "64a69817effe1f448a4053b4", // "Bluetooth",
    "64971d11be1a76c06747de2f", // "Games",
    "64971d106617ba37a4bc79b9", // "GPIO",
    "64971d106617ba37a4bc79b6", // "Infrared",
    "64971d11be1a76c06747de29", // "iButton",
    "64971d116617ba37a4bc79bc", // "Media",
    "64971d10be1a76c06747de26", // "NFC",
    "64971d10577d519190ede5c2", // "RFID",
    "64971d0f6617ba37a4bc79b3", // "Sub-GHz",
    "64971d11577d519190ede5c5", // "Tools",
    "64971d11be1a76c06747de2c", // "USB",
};

char *categories[] = {
    "Bluetooth", // "64a69817effe1f448a4053b4"
    "Games",     // "64971d11be1a76c06747de2f"
    "GPIO",      // "64971d106617ba37a4bc79b9"
    "Infrared",  // "64971d106617ba37a4bc79b6"
    "iButton",   // "64971d11be1a76c06747de29"
    "Media",     // "64971d116617ba37a4bc79bc"
    "NFC",       // "64971d10be1a76c06747de26"
    "RFID",      // "64971d10577d519190ede5c2"
    "Sub-GHz",   // "64971d0f6617ba37a4bc79b3"
    "Tools",     // "64971d11577d519190ede5c5"
    "USB",       // "64971d11be1a76c06747de2c"
};

FlipStoreAppInfo *flip_catalog_alloc()
{
    FlipStoreAppInfo *app_catalog = malloc(MAX_APP_COUNT * sizeof(FlipStoreAppInfo));
    if (!app_catalog)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for flip_catalog.");
        return NULL;
    }
    // No need for a loop since all memory is allocated in one block
    return app_catalog;
}

void flip_catalog_free()
{
    if (flip_catalog)
    {
        free(flip_catalog);
        flip_catalog = NULL;
    }
}

bool flip_store_process_app_list(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL.");
        return false;
    }
    // Initialize the flip_catalog
    flip_catalog = flip_catalog_alloc();
    if (!flip_catalog)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for flip_catalog.");
        return false;
    }

    FuriString *feed_data = flipper_http_load_from_file(fhttp->file_path);
    if (feed_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    FuriString *json_data_str = furi_string_alloc();
    if (!json_data_str)
    {
        FURI_LOG_E("Game", "Failed to allocate json_data string");
        return NULL;
    }

    furi_string_cat_str(json_data_str, "{\"json_data\":");
    furi_string_cat(json_data_str, feed_data);
    furi_string_free(feed_data);
    furi_string_cat_str(json_data_str, "}");

    int app_count = 0;

    // parse the JSON data
    for (int i = 0; i < MAX_APP_COUNT; i++)
    {
        FuriString *json_data_array = get_json_array_value_furi("json_data", i, json_data_str);
        if (!json_data_array)
        {
            FURI_LOG_I(TAG, "No more apps to process, i = %d", i);
            break;
        }

        FuriString *app_id = get_json_value_furi("alias", json_data_array); // app_id is stored in the "alias" field
        if (!app_id)
        {
            FURI_LOG_E(TAG, "Failed to get app_id.");
            break;
        }
        snprintf(flip_catalog[i].app_id, MAX_ID_LENGTH, "%s", furi_string_get_cstr(app_id));
        furi_string_free(app_id);

        FuriString *current_version = get_json_value_furi("current_version", json_data_array);
        if (!current_version)
        {
            FURI_LOG_E(TAG, "Failed to get current_version.");
            break;
        }

        FuriString *app_name = get_json_value_furi("name", current_version);
        if (!app_name)
        {
            FURI_LOG_E(TAG, "Failed to get app_name.");
            break;
        }
        snprintf(flip_catalog[i].app_name, MAX_APP_NAME_LENGTH, "%s", furi_string_get_cstr(app_name));
        furi_string_free(app_name);

        FuriString *app_description = get_json_value_furi("short_description", current_version);
        if (!app_description)
        {
            FURI_LOG_E(TAG, "Failed to get app_description.");
            break;
        }
        snprintf(flip_catalog[i].app_description, MAX_APP_DESCRIPTION_LENGTH, "%s", furi_string_get_cstr(app_description));
        furi_string_free(app_description);

        FuriString *app_version = get_json_value_furi("version", current_version);
        if (!app_version)
        {
            FURI_LOG_E(TAG, "Failed to get app_version.");
            break;
        }
        snprintf(flip_catalog[i].app_version, MAX_APP_VERSION_LENGTH, "%s", furi_string_get_cstr(app_version));
        furi_string_free(app_version);

        FuriString *_id = get_json_value_furi("_id", current_version);
        if (!_id)
        {
            FURI_LOG_E(TAG, "Failed to get _id.");
            break;
        }
        snprintf(flip_catalog[i].app_build_id, MAX_ID_LENGTH, "%s", furi_string_get_cstr(_id));
        furi_string_free(_id);

        app_count++;
        furi_string_free(json_data_array);
        furi_string_free(current_version);
    }

    furi_string_free(json_data_str);
    return app_count > 0;
}

bool flip_store_get_fap_file(FlipperHTTP *fhttp, char *build_id, uint8_t target, uint16_t api_major, uint16_t api_minor)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL.");
        return false;
    }
    if (!build_id)
    {
        FURI_LOG_E(TAG, "Build ID is NULL.");
        return false;
    }
    fhttp->state = IDLE;
    char url[128];
    fhttp->save_received_data = false;
    fhttp->is_bytes_request = true;
    snprintf(url, sizeof(url), "https://catalog.flipperzero.one/api/v0/application/version/%s/build/compatible?target=f%d&api=%d.%d", build_id, target, api_major, api_minor);
    return flipper_http_get_request_bytes(fhttp, url, "{\"Content-Type\": \"application/octet-stream\"}");
}

bool flip_store_install_app(FlipperHTTP *fhttp, char *category)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL.");
        return false;
    }
    // create /apps/FlipStore directory if it doesn't exist
    char directory_path[128];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps/%s", category);

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);
    furi_record_close(RECORD_STORAGE);

    snprintf(fhttp->file_path, sizeof(fhttp->file_path), STORAGE_EXT_PATH_PREFIX "/apps/%s/%s.fap", category, flip_catalog[app_selected_index].app_id);

    uint8_t target = furi_hal_version_get_hw_target();
    uint16_t api_major, api_minor;
    furi_hal_info_get_api_version(&api_major, &api_minor);
    if (fhttp->state != INACTIVE && flip_store_get_fap_file(fhttp, flip_catalog[app_selected_index].app_build_id, target, api_major, api_minor))
    {
        fhttp->state = RECEIVING;
        return true;
    }
    else
    {
        FURI_LOG_E(TAG, "Failed to send the request");
        flip_store_success = false;
        return false;
    }
}