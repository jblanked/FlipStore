#include <apps/flip_store_apps.h>

FlipStoreAppInfo *flip_catalog = NULL;

uint32_t app_selected_index = 0;
bool flip_store_sent_request = false;
bool flip_store_success = false;
bool flip_store_saved_data = false;
bool flip_store_saved_success = false;
uint32_t flip_store_category_index = 0;

// define the list of categories
char *categories[] = {
    "Bluetooth",
    "Games",
    "GPIO",
    "Infrared",
    "iButton",
    "Media",
    "NFC",
    "RFID",
    "Sub-GHz",
    "Tools",
    "USB",
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

    char *data_cstr = (char *)furi_string_get_cstr(feed_data);
    if (data_cstr == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get C-string from FuriString.");
        furi_string_free(feed_data);
        return false;
    }

    // Parser state variables
    bool in_string = false;
    bool is_escaped = false;
    bool reading_key = false;
    bool reading_value = false;
    bool inside_app_object = false;
    bool found_name = false, found_id = false, found_build_id = false, found_version = false, found_description = false;
    char current_key[MAX_KEY_LENGTH] = {0};
    size_t key_index = 0;
    char current_value[MAX_VALUE_LENGTH] = {0};
    size_t value_index = 0;
    int app_count = 0;
    enum ObjectState object_state = OBJECT_EXPECT_KEY;
    enum
    {
        STATE_SEARCH_APPS_KEY,
        STATE_SEARCH_ARRAY_START,
        STATE_READ_ARRAY_ELEMENTS,
        STATE_DONE
    } state = STATE_SEARCH_APPS_KEY;

    // Iterate through the data
    for (size_t i = 0; data_cstr[i] != '\0' && state != STATE_DONE; ++i)
    {
        char c = data_cstr[i];

        if (is_escaped)
        {
            is_escaped = false;
            if (reading_key && key_index < MAX_KEY_LENGTH - 1)
            {
                current_key[key_index++] = c;
            }
            else if (reading_value && value_index < MAX_VALUE_LENGTH - 1)
            {
                current_value[value_index++] = c;
            }
            continue;
        }

        if (c == '\\')
        {
            is_escaped = true;
            continue;
        }

        if (c == '\"')
        {
            in_string = !in_string;
            if (in_string)
            {
                if (!reading_key && !reading_value)
                {
                    if (state == STATE_SEARCH_APPS_KEY || object_state == OBJECT_EXPECT_KEY)
                    {
                        reading_key = true;
                        key_index = 0;
                        current_key[0] = '\0';
                    }
                    else if (object_state == OBJECT_EXPECT_VALUE)
                    {
                        reading_value = true;
                        value_index = 0;
                        current_value[0] = '\0';
                    }
                }
            }
            else
            {
                if (reading_key)
                {
                    reading_key = false;
                    current_key[key_index] = '\0';
                    if (state == STATE_SEARCH_APPS_KEY && strcmp(current_key, "apps") == 0)
                    {
                        state = STATE_SEARCH_ARRAY_START;
                    }
                    else if (inside_app_object)
                    {
                        object_state = OBJECT_EXPECT_COLON;
                    }
                }
                else if (reading_value)
                {
                    reading_value = false;
                    current_value[value_index] = '\0';

                    if (inside_app_object)
                    {
                        if (strcmp(current_key, "name") == 0)
                        {
                            snprintf(flip_catalog[app_count].app_name, MAX_APP_NAME_LENGTH, "%.31s", current_value);
                            found_name = true;
                        }
                        else if (strcmp(current_key, "id") == 0)
                        {
                            snprintf(flip_catalog[app_count].app_id, MAX_ID_LENGTH, "%.31s", current_value);
                            found_id = true;
                        }
                        else if (strcmp(current_key, "build_id") == 0)
                        {
                            snprintf(flip_catalog[app_count].app_build_id, MAX_ID_LENGTH, "%.31s", current_value);
                            found_build_id = true;
                        }
                        else if (strcmp(current_key, "version") == 0)
                        {
                            snprintf(flip_catalog[app_count].app_version, MAX_APP_VERSION_LENGTH, "%.3s", current_value);
                            found_version = true;
                        }
                        else if (strcmp(current_key, "description") == 0)
                        {
                            snprintf(flip_catalog[app_count].app_description, MAX_APP_DESCRIPTION_LENGTH, "%.99s", current_value);
                            found_description = true;
                        }

                        if (found_name && found_id && found_build_id && found_version && found_description)
                        {
                            app_count++;
                            if (app_count >= MAX_APP_COUNT)
                            {
                                FURI_LOG_I(TAG, "Reached maximum app count.");
                                state = STATE_DONE;
                                break;
                            }

                            found_name = found_id = found_build_id = found_version = found_description = false;
                        }

                        object_state = OBJECT_EXPECT_COMMA_OR_END;
                    }
                }
            }
            continue;
        }

        if (in_string)
        {
            if (reading_key && key_index < MAX_KEY_LENGTH - 1)
            {
                current_key[key_index++] = c;
            }
            else if (reading_value && value_index < MAX_VALUE_LENGTH - 1)
            {
                current_value[value_index++] = c;
            }
            continue;
        }

        if (state == STATE_SEARCH_ARRAY_START && c == '[')
        {
            state = STATE_READ_ARRAY_ELEMENTS;
            continue;
        }

        if (state == STATE_READ_ARRAY_ELEMENTS)
        {
            if (c == '{')
            {
                inside_app_object = true;
                object_state = OBJECT_EXPECT_KEY;
            }
            else if (c == '}')
            {
                inside_app_object = false;
            }
            else if (c == ':')
            {
                object_state = OBJECT_EXPECT_VALUE;
            }
            else if (c == ',')
            {
                object_state = OBJECT_EXPECT_KEY;
            }
            else if (c == ']')
            {
                state = STATE_DONE;
                break;
            }
        }
    }

    // Clean up
    furi_string_free(feed_data);
    free(data_cstr);
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