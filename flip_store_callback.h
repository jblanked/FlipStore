#ifndef FLIP_STORE_CALLBACK_H
#define FLIP_STORE_CALLBACK_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jsmn.h"

// Define maximum limits
#define MAX_APP_NAME_LENGTH 50
#define MAX_APP_COUNT 200

typedef struct
{
    char app_names[MAX_APP_COUNT][MAX_APP_NAME_LENGTH];
    char app_ids[MAX_APP_COUNT][MAX_APP_NAME_LENGTH];
} FlipStoreAppCatalog;

static FlipStoreAppCatalog flip_catalog;

uint32_t app_selected_index = 0;
bool flip_store_sent_request = false;
bool flip_store_success = false;
bool flip_store_saved_data = false;
bool flip_store_saved_success = false;

// Helper function to compare JSON keys
int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING &&
        (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}
// Function to clean app name string
void clean_app_name(char *name)
{
    // Remove leading and trailing whitespace (if needed)
    char *end;
    while (isspace((unsigned char)*name))
        name++; // Trim leading
    end = name + strlen(name) - 1;
    while (end > name && isspace((unsigned char)*end))
        end--;         // Trim trailing
    *(end + 1) = '\0'; // Null terminate
}

// Function to skip tokens correctly
int skip_tokens(jsmntok_t *tokens, int index, int total_tokens)
{
    int skip = 1; // Start with 1 to skip the current token
    int child_count = tokens[index].size;

    for (int i = 0; i < child_count; i++)
    {
        if ((index + skip) >= total_tokens)
            break;

        skip += skip_tokens(tokens, index + skip, total_tokens);
    }

    return skip;
}

bool flip_store_process_app_list(char *json_data)
{
    if (json_data == NULL)
    {
        FURI_LOG_E(TAG, "JSON data is NULL.");
        return false;
    }

    jsmn_parser parser;
    jsmn_init(&parser);

    // Initial token allocation
    int token_count = 128;
    jsmntok_t *tokens = (jsmntok_t *)malloc(sizeof(jsmntok_t) * token_count);
    if (tokens == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for JSON tokens.");
        return false;
    }

    int ret = jsmn_parse(&parser, json_data, strlen(json_data), tokens, token_count);

    // Reallocate tokens if needed
    while (ret == JSMN_ERROR_NOMEM)
    {
        token_count *= 2;
        jsmntok_t *new_tokens = (jsmntok_t *)realloc(tokens, sizeof(jsmntok_t) * token_count);
        if (new_tokens == NULL)
        {
            FURI_LOG_E(TAG, "Failed to reallocate memory for JSON tokens.");
            free(tokens);
            return false;
        }
        tokens = new_tokens;
        ret = jsmn_parse(&parser, json_data, strlen(json_data), tokens, token_count);
    }

    if (ret < 0)
    {
        FURI_LOG_E(TAG, "Failed to parse JSON: %d", ret);
        free(tokens);
        return false;
    }

    if (ret < 1 || tokens[0].type != JSMN_OBJECT)
    {
        FURI_LOG_E(TAG, "Root element is not an object.");
        free(tokens);
        return false;
    }

    int app_count = 0;
    int i = 1;

    while (i < ret)
    {
        if (jsoneq(json_data, &tokens[i], "apps") == 0)
        {
            jsmntok_t *apps_array = &tokens[i + 1];

            if (apps_array->type != JSMN_ARRAY)
            {
                FURI_LOG_E(TAG, "\"apps\" is not an array.");
                free(tokens);
                return false;
            }

            int current = i + 2;

            for (int j = 0; j < apps_array->size; j++)
            {
                if (current >= ret)
                {
                    FURI_LOG_E(TAG, "Token index out of bounds while accessing apps.");
                    break;
                }

                jsmntok_t *app_token = &tokens[current];

                if (app_token->type != JSMN_OBJECT)
                {
                    FURI_LOG_E(TAG, "App entry is not an object.");
                    current++;
                    continue;
                }

                int app_size = app_token->size;
                int app_token_index = current + 1;

                char name_value[MAX_APP_NAME_LENGTH] = {0};
                char id_value[MAX_APP_NAME_LENGTH] = {0};

                for (int k = 0; k < app_size; k++)
                {
                    if (app_token_index + 1 >= ret)
                    {
                        FURI_LOG_E(TAG, "Token index out of bounds while accessing app properties.");
                        break;
                    }

                    jsmntok_t *key_token = &tokens[app_token_index];
                    jsmntok_t *val_token = &tokens[app_token_index + 1];

                    int key_length = key_token->end - key_token->start;
                    if (key_length >= MAX_APP_NAME_LENGTH)
                        key_length = MAX_APP_NAME_LENGTH - 1;

                    char key_string[MAX_APP_NAME_LENGTH];
                    strncpy(key_string, json_data + key_token->start, key_length);
                    key_string[key_length] = '\0';

                    if (jsoneq(json_data, key_token, "name") == 0)
                    {
                        int val_length = val_token->end - val_token->start;
                        if (val_length >= MAX_APP_NAME_LENGTH)
                            val_length = MAX_APP_NAME_LENGTH - 1;
                        strncpy(name_value, json_data + val_token->start, val_length);
                        name_value[val_length] = '\0';
                    }
                    else if (jsoneq(json_data, key_token, "id") == 0)
                    {
                        int val_length = val_token->end - val_token->start;
                        if (val_length >= MAX_APP_NAME_LENGTH)
                            val_length = MAX_APP_NAME_LENGTH - 1;
                        strncpy(id_value, json_data + val_token->start, val_length);
                        id_value[val_length] = '\0';
                    }

                    app_token_index += 2;
                }

                if (app_count >= MAX_APP_COUNT)
                {
                    FURI_LOG_E(TAG, "Reached maximum app count limit.");
                    break;
                }

                strncpy(flip_catalog.app_names[app_count], name_value, MAX_APP_NAME_LENGTH - 1);
                flip_catalog.app_names[app_count][MAX_APP_NAME_LENGTH - 1] = '\0';

                strncpy(flip_catalog.app_ids[app_count], id_value, MAX_APP_NAME_LENGTH - 1);
                flip_catalog.app_ids[app_count][MAX_APP_NAME_LENGTH - 1] = '\0';

                app_count++;

                int tokens_to_skip = 1 + 2 * app_size;
                current += tokens_to_skip;
            }

            break;
        }
        else
        {
            i += 2;
        }
    }

    free(tokens);
    return true;
}

bool flip_store_get_fap_file(char *app_id)
{
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"app_id\":\"%s\"}", app_id);
    return flipper_http_post_request_bytes("https://www.flipsocial.net/api/app/compile/", "{\"Content-Type\":\"application/json\"}", payload);
}
void flip_store_request_error(Canvas *canvas)
{
    if (fhttp.received_data == NULL)
    {
        if (fhttp.last_response != NULL)
        {
            if (strstr(fhttp.last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
            else if (strstr(fhttp.last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
            else
            {
                FURI_LOG_E(TAG, "Received an error: %s", fhttp.last_response);
                canvas_draw_str(canvas, 0, 42, "Unusual error...");
                canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
        }
        else
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Unknown error.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
    }
    else
    {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 0, 10, "Failed to receive data.");
        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
    }
}
// function to handle the entire installation process "asynchronously"
bool flip_store_install_app(Canvas *canvas)
{
    // create /apps/FlipStore directory if it doesn't exist
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps/FlipStore");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);

    char *app_name = flip_catalog.app_names[app_selected_index];
    char installaing_text[128];
    snprintf(installaing_text, sizeof(installaing_text), "Installing %s", app_name);
    char bin_path[256];
    snprintf(bin_path, sizeof(bin_path), STORAGE_EXT_PATH_PREFIX "/apps/FlipStore/%s.fap", flip_catalog.app_ids[app_selected_index]);
    strncpy(fhttp.file_path, bin_path, sizeof(fhttp.file_path) - 1);
    canvas_draw_str(canvas, 0, 10, installaing_text);
    canvas_draw_str(canvas, 0, 20, "Sending reqeuest..");
    if (fhttp.state != INACTIVE && flip_store_get_fap_file(flip_catalog.app_ids[app_selected_index]))
    {
        canvas_draw_str(canvas, 0, 30, "Request sent.");
        fhttp.state = RECEIVING;
        // furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        canvas_draw_str(canvas, 0, 40, "Receiving...");
    }
    else
    {
        FURI_LOG_E(TAG, "Failed to send the request");
        flip_store_success = false;
        return false;
    }
    while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
    {
        // Wait for the feed to be received
        // furi_delay_ms(100);
    }
    // furi_timer_stop(fhttp.get_timeout_timer);
    if (fhttp.state == ISSUE || fhttp.received_data == NULL)
    {
        flip_store_request_error(canvas);
        flip_store_success = false;
        return false;
    }
    flip_store_success = true;
    return true;
}

// Callback for drawing the main screen
static void flip_store_view_draw_callback_main(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_set_font(canvas, FontSecondary);

    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your Dev Board with the");
        canvas_draw_str(canvas, 0, 62, "FlipperHTTP firmware.");
        return;
    }

    if (!flip_store_sent_request)
    {
        flip_store_sent_request = true;

        if (!flip_store_install_app(canvas))
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Failed to install app.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "App installed successfully.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
    }
    else
    {
        if (flip_store_success)
        {

            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "App installed successfully.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Failed to install app.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
    }
}

static void flip_store_view_draw_callback_app_list(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, flip_catalog.app_names[app_selected_index]);
    // canvas_draw_icon(canvas, 0, 53, &I_ButtonLeft_4x7); (future implementation)
    //  canvas_draw_str_aligned(canvas, 7, 54, AlignLeft, AlignTop, "Delete");  (future implementation)
    canvas_draw_icon(canvas, 0, 53, &I_ButtonBACK_10x8);
    canvas_draw_str_aligned(canvas, 12, 54, AlignLeft, AlignTop, "Back");
    canvas_draw_icon(canvas, 90, 53, &I_ButtonRight_4x7);
    canvas_draw_str_aligned(canvas, 97, 54, AlignLeft, AlignTop, "Install");
}

static bool flip_store_input_callback(InputEvent *event, void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }
    if (event->type == InputTypeShort)
    {
        // Future implementation
        // if (event->key == InputKeyLeft)
        //{
        // Left button clicked, delete the app with DialogEx confirmation
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppDelete);
        //    return true;
        //}
        if (event->key == InputKeyRight)
        {
            // Right button clicked, download the app
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewMain);
            return true;
        }
    }
    else if (event->type == InputTypePress)
    {
        if (event->key == InputKeyBack)
        {
            // Back button clicked, switch to the previous view.
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
            return true;
        }
    }

    return false;
}

static void flip_store_text_updated_ssid(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // store the entered text
    strncpy(app->uart_text_input_buffer_ssid, app->uart_text_input_temp_buffer_ssid, app->uart_text_input_buffer_size_ssid);

    // Ensure null-termination
    app->uart_text_input_buffer_ssid[app->uart_text_input_buffer_size_ssid - 1] = '\0';

    // update the variable item text
    if (app->variable_item_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_ssid, app->uart_text_input_buffer_ssid);
    }

    // save the settings
    save_settings(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_pass);

    // if SSID and PASS are not empty, connect to the WiFi
    if (strlen(app->uart_text_input_buffer_ssid) > 0 && strlen(app->uart_text_input_buffer_pass) > 0)
    {
        // save wifi settings
        if (!flipper_http_save_wifi(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_pass))
        {
            FURI_LOG_E(TAG, "Failed to save WiFi settings");
        }
    }

    // switch to the settings view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
}
static void flip_store_text_updated_pass(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // store the entered text
    strncpy(app->uart_text_input_buffer_pass, app->uart_text_input_temp_buffer_pass, app->uart_text_input_buffer_size_pass);

    // Ensure null-termination
    app->uart_text_input_buffer_pass[app->uart_text_input_buffer_size_pass - 1] = '\0';

    // update the variable item text
    if (app->variable_item_pass)
    {
        variable_item_set_current_value_text(app->variable_item_pass, app->uart_text_input_buffer_pass);
    }

    // save the settings
    save_settings(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_pass);

    // if SSID and PASS are not empty, connect to the WiFi
    if (strlen(app->uart_text_input_buffer_ssid) > 0 && strlen(app->uart_text_input_buffer_pass) > 0)
    {
        // save wifi settings
        if (!flipper_http_save_wifi(app->uart_text_input_buffer_ssid, app->uart_text_input_buffer_pass))
        {
            FURI_LOG_E(TAG, "Failed to save WiFi settings");
        }
    }

    // switch to the settings view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
}

static uint32_t callback_to_submenu(void *context)
{
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    return FlipStoreViewSubmenu;
}

static uint32_t callback_to_app_list(void *context)
{
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    flip_store_sent_request = false;
    flip_store_success = false;
    flip_store_saved_data = false;
    flip_store_saved_success = false;
    return FlipStoreViewAppList;
}

static void settings_item_selected(void *context, uint32_t index)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input SSID
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInputSSID);
        break;
    case 1: // Input Password
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInputPass);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

void dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (result == DialogExResultLeft) // No
    {
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
    }
    else if (result == DialogExResultRight)
    {
        // delete the app then return to the app list

        // pop up a message
        popup_set_header(app->popup, "Success", 0, 0, AlignLeft, AlignTop);
        popup_set_text(app->popup, "App deleted successfully.", 0, 60, AlignLeft, AlignTop);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
        furi_delay_ms(2000); // delay for 2 seconds
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
    }
}

void popup_callback(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenu);
}

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
static uint32_t callback_exit_app(void *context)
{
    // Exit the application
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    return VIEW_NONE; // Return VIEW_NONE to exit the app
}

static void callback_submenu_choices(void *context, uint32_t index)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    switch (index)
    {
    case FlipStoreSubmenuIndexMain:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewMain);
        break;
    case FlipStoreSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAbout);
        break;
    case FlipStoreSubmenuIndexSettings:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
        break;
    case FlipStoreSubmenuIndexDownloadApp:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
        break;
    // Ideally users should be sent to a draw callback view to show to request process (like in FlipSocial and WebCrawler)
    case FlipStoreSubmenuIndexAppList:
        // async call to the app list with timer
        if (fhttp.state != INACTIVE && flipper_http_get_request_with_headers("https://www.flipsocial.net/api/flipper/apps/", "{\"Content-Type\":\"application/json\"}"))
        {
            furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
            fhttp.state = RECEIVING;
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to send the request");
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
            return;
        }
        while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
        {
            // Wait for the feed to be received
            furi_delay_ms(100);
        }
        furi_timer_stop(fhttp.get_timeout_timer);

        if (fhttp.state == ISSUE || fhttp.received_data == NULL)
        {
            if (fhttp.received_data == NULL)
            {
                FURI_LOG_E(TAG, "Failed to receive data");
                if (fhttp.last_response != NULL)
                {
                    if (strstr(fhttp.last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
                    {
                        popup_set_text(app->popup, "[ERROR] WiFi Disconnected.\n\n\nUpdate your WiFi settings.\nPress BACK to return.", 0, 10, AlignLeft, AlignTop);
                    }
                    else if (strstr(fhttp.last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
                    {
                        popup_set_text(app->popup, "[ERROR] WiFi Disconnected.\n\n\nUpdate your WiFi settings.\nPress BACK to return.", 0, 10, AlignLeft, AlignTop);
                    }
                    else
                    {
                        popup_set_text(app->popup, fhttp.last_response, 0, 10, AlignLeft, AlignTop);
                    }
                }
                else
                {
                    popup_set_text(app->popup, "[ERROR] Unknown Error.\n\n\nUpdate your WiFi settings.\nPress BACK to return.", 0, 10, AlignLeft, AlignTop);
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
                return;
            }
            else
            {
                FURI_LOG_E(TAG, "Failed to receive data");
                popup_set_text(app->popup, "Failed to received data.", 0, 10, AlignLeft, AlignTop);
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
                return;
            }
        }
        else
        {
            // process the app list
            if (flip_store_process_app_list(fhttp.received_data))
            {
                submenu_reset(app->submenu_app_list); // clear the submenu
                // add each app name to submenu
                for (int i = 0; i < MAX_APP_COUNT; i++)
                {
                    if (strlen(flip_catalog.app_names[i]) > 0)
                    {
                        submenu_add_item(app->submenu_app_list, flip_catalog.app_names[i], FlipStoreSubmenuIndexStartAppList + i, callback_submenu_choices, app);
                    }
                }

                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
            }
            else
            {
                FURI_LOG_E(TAG, "Failed to process the app list");
                popup_set_text(app->popup, "Failed to process the app list", 0, 10, AlignLeft, AlignTop);
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
                return;
            }
        }
        break;
    default:
        // Check if the index is within the app list range
        if (index >= FlipStoreSubmenuIndexStartAppList && index < FlipStoreSubmenuIndexStartAppList + MAX_APP_COUNT)
        {
            // Get the app index
            uint32_t app_index = index - FlipStoreSubmenuIndexStartAppList;

            // Check if the app index is valid
            if ((int)app_index >= 0 && app_index < MAX_APP_COUNT)
            {
                // Get the app name
                char *app_name = flip_catalog.app_names[app_index];

                // Check if the app name is valid
                if (app_name != NULL && strlen(app_name) > 0)
                {
                    app_selected_index = app_index;
                    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppInfo);
                }
                else
                {
                    FURI_LOG_E(TAG, "Invalid app name");
                }
            }
            else
            {
                FURI_LOG_E(TAG, "Invalid app index");
            }
        }
        else
        {
            FURI_LOG_E(TAG, "Unknown submenu index");
        }
        break;
    }
}

#endif // FLIP_STORE_CALLBACK_H