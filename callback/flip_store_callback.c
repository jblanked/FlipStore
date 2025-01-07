#include <callback/flip_store_callback.h>
#include <github/flip_store_github.h>

// Below added by Derek Jamison
// FURI_LOG_DEV will log only during app development. Be sure that Settings/System/Log Device is "LPUART"; so we dont use serial port.
#ifdef DEVELOPMENT
#define FURI_LOG_DEV(tag, format, ...) furi_log_print_format(FuriLogLevelInfo, tag, format, ##__VA_ARGS__)
#define DEV_CRASH() furi_crash()
#else
#define FURI_LOG_DEV(tag, format, ...)
#define DEV_CRASH()
#endif

bool flip_store_app_does_exist = false;
uint32_t selected_firmware_index = 0;
static uint32_t callback_to_app_category_list(void *context);

static bool flip_store_dl_app_fetch(DataLoaderModel *model)
{
    if (!model->fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    return flip_store_install_app(model->fhttp, categories[flip_store_category_index]);
}
static char *flip_store_dl_app_parse(DataLoaderModel *model)
{
    if (!model->fhttp || model->fhttp->state != IDLE)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL or not IDLE");
        return "Failed to install app.";
    }
    return "App installed successfully.";
}
static void flip_store_dl_app_switch_to_view(FlipStoreApp *app)
{
    flip_store_generic_switch_to_view(app, flip_catalog[app_selected_index].app_name, flip_store_dl_app_fetch, flip_store_dl_app_parse, 1, callback_to_app_category_list, FlipStoreViewLoader);
}
//
static bool flip_store_fetch_firmware(DataLoaderModel *model)
{
    if (!model->fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    model->fhttp->state = IDLE;
    if (model->request_index == 0)
    {
        firmware_free();
        firmwares = firmware_alloc();
        if (!firmwares)
        {
            return false;
        }
        return flip_store_get_firmware_file(
            model->fhttp,
            firmwares[selected_firmware_index].links[0],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[0], '/') + 1);
    }
    else if (model->request_index == 1)
    {
        return flip_store_get_firmware_file(
            model->fhttp,
            firmwares[selected_firmware_index].links[1],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[1], '/') + 1);
    }
    else if (model->request_index == 2)
    {
        return flip_store_get_firmware_file(
            model->fhttp,
            firmwares[selected_firmware_index].links[2],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[2], '/') + 1);
    }
    return false;
}
static char *flip_store_parse_firmware(DataLoaderModel *model)
{
    if (model->request_index == 0)
    {
        return "File 1 installed.";
    }
    else if (model->request_index == 1)
    {
        return "File 2 installed.";
    }
    else if (model->request_index == 2)
    {
        return "Firmware downloaded successfully";
    }
    return "Failed to download firmware.";
}
static void flip_store_switch_to_firmware_list(FlipStoreApp *app)
{
    flip_store_generic_switch_to_view(app, firmwares[selected_firmware_index].name, flip_store_fetch_firmware, flip_store_parse_firmware, FIRMWARE_LINKS, callback_to_firmware_list, FlipStoreViewLoader);
}
//
static bool flip_store_fetch_vgm_firmware(DataLoaderModel *model)
{
    if (!model->fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    model->fhttp->state = IDLE;
    if (model->request_index == 0)
    {
        vgm_firmware_free();
        vgm_firmwares = vgm_firmware_alloc();
        if (!vgm_firmwares)
        {
            return false;
        }
        return flip_store_get_firmware_file(
            model->fhttp,
            vgm_firmwares[selected_firmware_index].link,
            vgm_firmwares[selected_firmware_index].name,
            strrchr(vgm_firmwares[selected_firmware_index].link, '/') + 1);
    }
    return false;
}
static char *flip_store_parse_vgm_firmware(DataLoaderModel *model)
{
    UNUSED(model);
    return "Firmware downloaded successfully";
}
static void flip_store_switch_to_vgm_firmware_list(FlipStoreApp *app)
{
    flip_store_generic_switch_to_view(app, vgm_firmwares[selected_firmware_index].name, flip_store_fetch_vgm_firmware, flip_store_parse_vgm_firmware, 1, callback_to_vgm_firmware_list, FlipStoreViewLoader);
}

// Function to draw the message on the canvas with word wrapping
static void draw_description(Canvas *canvas, const char *description, int x, int y)
{
    if (description == NULL || strlen(description) == 0)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return;
    }
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL.");
        return;
    }

    size_t msg_length = strlen(description);
    size_t start = 0;
    int line_num = 0;
    char line[MAX_LINE_LENGTH + 1]; // Buffer for the current line (+1 for null terminator)

    while (start < msg_length && line_num < 4)
    {
        size_t remaining = msg_length - start;
        size_t len = (remaining > MAX_LINE_LENGTH) ? MAX_LINE_LENGTH : remaining;

        if (remaining > MAX_LINE_LENGTH)
        {
            // Find the last space within the first 'len' characters
            size_t last_space = len;
            while (last_space > 0 && description[start + last_space - 1] != ' ')
            {
                last_space--;
            }

            if (last_space > 0)
            {
                len = last_space; // Adjust len to the position of the last space
            }
        }

        // Copy the substring to 'line' and null-terminate it
        memcpy(line, description + start, len);
        line[len] = '\0'; // Ensure the string is null-terminated

        // Draw the string on the canvas
        // Adjust the y-coordinate based on the line number
        canvas_draw_str_aligned(canvas, x, y + line_num * 10, AlignLeft, AlignTop, line);

        // Update the start position for the next line
        start += len;

        // Skip any spaces to avoid leading spaces on the next line
        while (start < msg_length && description[start] == ' ')
        {
            start++;
        }

        // Increment the line number
        line_num++;
    }
}

static void flip_store_view_draw_callback_app_list(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    char title[64];
    snprintf(title, 64, "%s (v.%s)", flip_catalog[app_selected_index].app_name, flip_catalog[app_selected_index].app_version);
    canvas_draw_str(canvas, 0, 10, title);
    canvas_set_font(canvas, FontSecondary);
    draw_description(canvas, flip_catalog[app_selected_index].app_description, 0, 13);
    if (flip_store_app_does_exist)
    {
        canvas_draw_icon(canvas, 0, 53, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 7, 54, AlignLeft, AlignTop, "Delete");
        canvas_draw_icon(canvas, 45, 53, &I_ButtonBACK_10x8);
        canvas_draw_str_aligned(canvas, 57, 54, AlignLeft, AlignTop, "Back");
    }
    else
    {
        canvas_draw_icon(canvas, 0, 53, &I_ButtonBACK_10x8);
        canvas_draw_str_aligned(canvas, 12, 54, AlignLeft, AlignTop, "Back");
    }
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
        if (event->key == InputKeyLeft && flip_store_app_does_exist)
        {
            // Left button clicked, delete the app
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppDelete);
            return true;
        }
        if (event->key == InputKeyRight)
        {
            // Right button clicked, download the app
            flip_store_dl_app_switch_to_view(app);
            return true;
        }
    }
    else if (event->type == InputTypePress)
    {
        if (event->key == InputKeyBack)
        {
            // Back button clicked, switch to the previous view.
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppListCategory);
            return true;
        }
    }

    return false;
}
static void free_text_input_view(FlipStoreApp *app);
static bool alloc_text_input_view(void *context, char *title);
static void flip_store_text_updated_ssid(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // store the entered text
    strncpy(app->uart_text_input_buffer, app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size);

    // Ensure null-termination
    app->uart_text_input_buffer[app->uart_text_input_buffer_size - 1] = '\0';

    // save the setting
    save_char("WiFi-SSID", app->uart_text_input_buffer);

    // update the variable item text
    if (app->variable_item_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_ssid, app->uart_text_input_buffer);

        // get value of password
        char pass[64];
        if (load_char("WiFi-Password", pass, sizeof(pass)))
        {
            if (strlen(pass) > 0 && strlen(app->uart_text_input_buffer) > 0)
            {
                // save the settings
                save_settings(app->uart_text_input_buffer, pass);

                // initialize the http
                FlipperHTTP *fhttp = flipper_http_alloc();
                if (fhttp)
                {
                    // save the wifi if the device is connected
                    if (!flipper_http_save_wifi(fhttp, app->uart_text_input_buffer, pass))
                    {
                        easy_flipper_dialog("FlipperHTTP Error", "Ensure your WiFi Developer\nBoard or Pico W is connected\nand the latest FlipperHTTP\nfirmware is installed.");
                    }

                    // free the resources
                    flipper_http_free(fhttp);
                }
                else
                {
                    easy_flipper_dialog("FlipperHTTP Error", "The UART is likely busy.\nEnsure you have the correct\nflash for your board then\nrestart your Flipper Zero.");
                }
            }
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
    strncpy(app->uart_text_input_buffer, app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size);

    // Ensure null-termination
    app->uart_text_input_buffer[app->uart_text_input_buffer_size - 1] = '\0';

    // save the setting
    save_char("WiFi-Password", app->uart_text_input_buffer);

    // update the variable item text
    if (app->variable_item_pass)
    {
        // variable_item_set_current_value_text(app->variable_item_pass, app->uart_text_input_buffer);
    }

    // get value of ssid
    char ssid[64];
    if (load_char("WiFi-SSID", ssid, sizeof(ssid)))
    {
        if (strlen(ssid) > 0 && strlen(app->uart_text_input_buffer) > 0)
        {
            // save the settings
            save_settings(ssid, app->uart_text_input_buffer);

            // initialize the http
            FlipperHTTP *fhttp = flipper_http_alloc();
            if (fhttp)
            {
                // save the wifi if the device is connected
                if (!flipper_http_save_wifi(fhttp, ssid, app->uart_text_input_buffer))
                {
                    easy_flipper_dialog("FlipperHTTP Error", "Ensure your WiFi Developer\nBoard or Pico W is connected\nand the latest FlipperHTTP\nfirmware is installed.");
                }

                // free the resources
                flipper_http_free(fhttp);
            }
            else
            {
                easy_flipper_dialog("FlipperHTTP Error", "The UART is likely busy.\nEnsure you have the correct\nflash for your board then\nrestart your Flipper Zero.");
            }
        }
    }

    // switch to the settings view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
}
static void flip_store_text_updated_repo(void *context);
static void flip_store_text_updated_author(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // store the entered text
    strncpy(app->uart_text_input_buffer, app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size);

    // Ensure null-termination
    app->uart_text_input_buffer[app->uart_text_input_buffer_size - 1] = '\0';

    // save the setting
    save_char("Github-Author", app->uart_text_input_buffer);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenu);
    uart_text_input_reset(app->uart_text_input);
    uart_text_input_set_header_text(app->uart_text_input, "Repository");
    app->uart_text_input_buffer_size = 64;
    free(app->uart_text_input_buffer);
    free(app->uart_text_input_temp_buffer);
    easy_flipper_set_buffer(&app->uart_text_input_buffer, app->uart_text_input_buffer_size);
    easy_flipper_set_buffer(&app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size);
    uart_text_input_set_result_callback(app->uart_text_input, flip_store_text_updated_repo, app, app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size, false);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInput);
}

static bool flip_store_fetch_github(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    char author[64];
    char repo[64];
    if (!load_char("Github-Author", author, sizeof(author)) || !load_char("Github-Repo", repo, sizeof(repo)))
    {
        FURI_LOG_E(TAG, "Failed to load Github author or repo");
        return false;
    }
    return flip_store_get_github_contents(fhttp, author, repo);
}

static bool flip_store_parse_github(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    char author[64];
    char repo[64];
    if (!load_char("Github-Author", author, sizeof(author)) || !load_char("Github-Repo", repo, sizeof(repo)))
    {
        FURI_LOG_E(TAG, "Failed to load Github author or repo");
        return false;
    }
    if (!flip_store_parse_github_contents(fhttp->file_path, author, repo))
    {
        return false;
    }
    return flip_store_install_all_github_files(fhttp, author, repo);
}
static bool github_success = false;
static void flip_store_get_github_repository(FlipStoreApp *app)
{
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
        return;
    }
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    bool http_request()
    {
        github_success = flip_store_fetch_github(fhttp);
        return github_success;
    }
    bool http_parse()
    {
        github_success = flip_store_parse_github(fhttp);
        return github_success;
    }
    flipper_http_loading_task(fhttp, http_request, http_parse, FlipStoreViewSubmenuOptions, FlipStoreViewWidgetResult, &app->view_dispatcher);
    flipper_http_free(fhttp);
    if (github_success)
    {
        easy_flipper_dialog("Success", "Repository downloaded\nsuccessfully.");
    }
    else
    {
        easy_flipper_dialog("Failure", "Failed to download\nrepository.");
    }
}
static void flip_store_text_updated_repo(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // store the entered text
    strncpy(app->uart_text_input_buffer, app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size);

    // Ensure null-termination
    app->uart_text_input_buffer[app->uart_text_input_buffer_size - 1] = '\0';

    // save the setting
    save_char("Github-Repo", app->uart_text_input_buffer);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenuOptions);
    flip_store_get_github_repository(app);
}
static void free_category_submenu(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    if (app->submenu_app_list_category)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewAppListCategory);
        submenu_free(app->submenu_app_list_category);
        app->submenu_app_list_category = NULL;
    }
}
static void free_variable_item_list(FlipStoreApp *app);

uint32_t callback_to_submenu(void *context)
{
    UNUSED(context);
    firmware_free();
    return FlipStoreViewSubmenu;
}

uint32_t callback_to_firmware_list(void *context)
{
    UNUSED(context);
    return FlipStoreViewFirmwares;
}
uint32_t callback_to_vgm_firmware_list(void *context)
{
    UNUSED(context);
    return FlipStoreViewVGMFirmwares;
}
static uint32_t callback_to_app_category_list(void *context)
{
    UNUSED(context);
    return FlipStoreViewAppListCategory;
}
uint32_t callback_to_app_list(void *context)
{
    UNUSED(context);
    return FlipStoreViewAppList;
}

static uint32_t callback_to_wifi_settings(void *context)
{
    UNUSED(context);
    return FlipStoreViewSettings;
}
static void dialog_firmware_callback(DialogExResult result, void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    if (result == DialogExResultLeft) // No
    {
        // switch to the firmware list
        if (is_esp32_firmware)
        {
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwares);
        }
        else
        {
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewVGMFirmwares);
        }
    }
    else if (result == DialogExResultRight)
    {
        // download the firmware then return to the firmware list
        if (is_esp32_firmware)
        {
            flip_store_switch_to_firmware_list(app);
        }
        else
        {
            flip_store_switch_to_vgm_firmware_list(app);
        }
    }
}

static bool alloc_about_view(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }
    if (!app->widget_about)
    {
        if (!easy_flipper_set_widget(
                &app->widget_about,
                FlipStoreViewAbout,
                "Welcome to the FlipStore!\n------\nDownload apps via WiFi and\nrun them on your Flipper!\n------\nwww.github.com/jblanked",
                callback_to_submenu,
                &app->view_dispatcher))
        {
            return false;
        }
        if (!app->widget_about)
        {
            return false;
        }
    }
    return true;
}

static void free_about_view(FlipStoreApp *app)
{
    if (app && app->widget_about != NULL)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewAbout);
        widget_free(app->widget_about);
        app->widget_about = NULL;
    }
}
static bool alloc_text_input_view(void *context, char *title)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }
    if (!title)
    {
        FURI_LOG_E(TAG, "Title is NULL");
        return false;
    }
    app->uart_text_input_buffer_size = 64;
    if (!app->uart_text_input_buffer)
    {
        if (!easy_flipper_set_buffer(&app->uart_text_input_buffer, app->uart_text_input_buffer_size))
        {
            return false;
        }
    }
    if (!app->uart_text_input_temp_buffer)
    {
        if (!easy_flipper_set_buffer(&app->uart_text_input_temp_buffer, app->uart_text_input_buffer_size))
        {
            return false;
        }
    }
    if (!app->uart_text_input)
    {
        if (strcmp(title, "SSID") != 0 && strcmp(title, "Password") != 0)
        {
            // Github repository download
            if (!easy_flipper_set_uart_text_input(
                    &app->uart_text_input,
                    FlipStoreViewTextInput,
                    title,
                    app->uart_text_input_temp_buffer,
                    app->uart_text_input_buffer_size,
                    strcmp(title, "Author") == 0 ? flip_store_text_updated_author : flip_store_text_updated_repo,
                    callback_to_submenu_options,
                    &app->view_dispatcher,
                    app))
            {
                return false;
            }
            if (!app->uart_text_input)
            {
                return false;
            }
        }
        else
        {
            if (!easy_flipper_set_uart_text_input(
                    &app->uart_text_input,
                    FlipStoreViewTextInput,
                    title,
                    app->uart_text_input_temp_buffer,
                    app->uart_text_input_buffer_size,
                    strcmp(title, "SSID") == 0 ? flip_store_text_updated_ssid : flip_store_text_updated_pass,
                    callback_to_wifi_settings,
                    &app->view_dispatcher,
                    app))
            {
                return false;
            }
            if (!app->uart_text_input)
            {
                return false;
            }
            char ssid[64];
            char pass[64];
            if (load_settings(ssid, sizeof(ssid), pass, sizeof(pass)))
            {
                if (strcmp(title, "SSID") == 0)
                {
                    strncpy(app->uart_text_input_temp_buffer, ssid, app->uart_text_input_buffer_size);
                }
                else
                {
                    strncpy(app->uart_text_input_temp_buffer, pass, app->uart_text_input_buffer_size);
                }
            }
        }
    }
    return true;
}
static void free_text_input_view(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    if (app->uart_text_input)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewTextInput);
        uart_text_input_free(app->uart_text_input);
        app->uart_text_input = NULL;
    }
    if (app->uart_text_input_buffer)
    {
        free(app->uart_text_input_buffer);
        app->uart_text_input_buffer = NULL;
    }
    if (app->uart_text_input_temp_buffer)
    {
        free(app->uart_text_input_temp_buffer);
        app->uart_text_input_temp_buffer = NULL;
    }
}
static void settings_item_selected(void *context, uint32_t index);
static bool alloc_variable_item_list(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }
    if (!app->variable_item_list)
    {
        if (!easy_flipper_set_variable_item_list(&app->variable_item_list, FlipStoreViewSettings, settings_item_selected, callback_to_submenu, &app->view_dispatcher, app))
            return false;

        if (!app->variable_item_list)
            return false;

        if (!app->variable_item_ssid)
        {
            app->variable_item_ssid = variable_item_list_add(app->variable_item_list, "SSID", 0, NULL, NULL);
            variable_item_set_current_value_text(app->variable_item_ssid, "");
        }
        if (!app->variable_item_pass)
        {
            app->variable_item_pass = variable_item_list_add(app->variable_item_list, "Password", 0, NULL, NULL);
            variable_item_set_current_value_text(app->variable_item_pass, "");
        }
        char ssid[64];
        char pass[64];
        if (load_settings(ssid, sizeof(ssid), pass, sizeof(pass)))
        {
            variable_item_set_current_value_text(app->variable_item_ssid, ssid);
            // variable_item_set_current_value_text(app->variable_item_pass, pass);
            save_char("WiFi-SSID", ssid);
            save_char("WiFi-Password", pass);
        }
    }
    return true;
}
static void free_variable_item_list(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    if (app->variable_item_list)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewSettings);
        variable_item_list_free(app->variable_item_list);
        app->variable_item_list = NULL;
    }
    if (app->variable_item_ssid)
    {
        free(app->variable_item_ssid);
        app->variable_item_ssid = NULL;
    }
    if (app->variable_item_pass)
    {
        free(app->variable_item_pass);
        app->variable_item_pass = NULL;
    }
}
static bool alloc_dialog_firmware(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }

    if (!app->dialog_firmware)
    {
        if (!easy_flipper_set_dialog_ex(
                &app->dialog_firmware,
                FlipStoreViewFirmwareDialog,
                is_esp32_firmware ? "Download ESP32 Firmware" : "Download VGM Firmware",
                0,
                0,
                "Are you sure you want to\ndownload this firmware?",
                0,
                10,
                "No",
                "Yes",
                NULL,
                dialog_firmware_callback,
                callback_to_firmware_list,
                &app->view_dispatcher,
                app))
        {
            return false;
        }
        if (!app->dialog_firmware)
        {
            return false;
        }
        if (is_esp32_firmware)
        {
            dialog_ex_set_header(app->dialog_firmware, firmwares[selected_firmware_index].name, 0, 0, AlignLeft, AlignTop);
        }
        else
        {
            dialog_ex_set_header(app->dialog_firmware, vgm_firmwares[selected_firmware_index].name, 0, 0, AlignLeft, AlignTop);
        }
    }
    return true;
}
static void free_dialog_firmware(FlipStoreApp *app)
{
    if (app && app->dialog_firmware)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewFirmwareDialog);
        dialog_ex_free(app->dialog_firmware);
        app->dialog_firmware = NULL;
    }
}
static bool alloc_app_info_view(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return false;
    }
    if (!app->view_app_info)
    {
        if (!easy_flipper_set_view(
                &app->view_app_info,
                FlipStoreViewAppInfo,
                flip_store_view_draw_callback_app_list,
                flip_store_input_callback,
                callback_to_app_category_list,
                &app->view_dispatcher,
                app))
        {
            return false;
        }
        if (!app->view_app_info)
        {
            return false;
        }
    }
    return true;
}
static void free_app_info_view(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    if (app->view_app_info)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewAppInfo);
        view_free(app->view_app_info);
        app->view_app_info = NULL;
    }
}
uint32_t callback_to_submenu_options(void *context)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return FlipStoreViewSubmenuOptions;
    }
    firmware_free();
    vgm_firmware_free();
    flip_catalog_free();
    free_category_submenu(app);
    return FlipStoreViewSubmenuOptions;
}
void free_all_views(FlipStoreApp *app, bool should_free_variable_item_list)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    free_about_view(app);
    flip_catalog_free();
    if (should_free_variable_item_list)
    {
        free_variable_item_list(app);
    }
    free_category_submenu(app);
    free_text_input_view(app);
    free_dialog_firmware(app);
    free_app_info_view(app);
    firmware_free();
    vgm_firmware_free();
}
uint32_t callback_exit_app(void *context)
{
    UNUSED(context);
    return VIEW_NONE; // Return VIEW_NONE to exit the app
}

static bool set_appropriate_list(FlipperHTTP *fhttp, FlipStoreApp *app)
{
    if (!fhttp || !app)
    {
        FURI_LOG_E(TAG, "FlipperHTTP oor app is NULL");
        return false;
    }

    if (!easy_flipper_set_submenu(&app->submenu_app_list_category, FlipStoreViewAppListCategory, categories[flip_store_category_index], callback_to_app_list, &app->view_dispatcher))
    {
        FURI_LOG_E(TAG, "Failed to set submenu");
        return false;
    }

    if (flip_store_process_app_list(fhttp) && app->submenu_app_list_category && flip_catalog)
    {
        submenu_reset(app->submenu_app_list_category);
        submenu_set_header(app->submenu_app_list_category, categories[flip_store_category_index]);
        // add each app name to submenu
        for (size_t i = 0; i < flip_catalog->count; i++)
        {
            if (strlen(flip_catalog[i].app_name) > 0)
            {
                submenu_add_item(app->submenu_app_list_category, flip_catalog[i].app_name, FlipStoreSubmenuIndexStartAppList + i, callback_submenu_choices, app);
            }
            else
            {
                break;
            }
        }
        // add [LOAD NEXT] to submenu
        submenu_add_item(app->submenu_app_list_category, "[LOAD NEXT]", FlipStoreSubmenuIndexStartAppList + flip_catalog->count, callback_submenu_choices, app);
        return true;
    }
    FURI_LOG_E(TAG, "Failed to process the app list");
    return false;
}

// we'll have to loop 8 at a time due to memory constraints
static void fetch_appropiate_app_list(FlipStoreApp *app, int iteration)
{
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
        return;
    }
    bool fetch_app_list()
    {
        // ensure /apps_data/flip_store/data exists
        Storage *storage = furi_record_open(RECORD_STORAGE);
        char dir[256];
        snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data");
        storage_common_mkdir(storage, dir);
        furi_record_close(RECORD_STORAGE);
        fhttp->state = IDLE;
        flip_catalog_free();
        snprintf(
            fhttp->file_path,
            sizeof(fhttp->file_path),
            STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s.json", categories[flip_store_category_index]);
        fhttp->save_received_data = true;
        fhttp->is_bytes_request = false;
        char url[256];
        // load 8 at a time
        snprintf(url, sizeof(url), "https://catalog.flipperzero.one/api/v0/0/application?limit=8&is_latest_release_version=true&offset=%d&sort_by=updated_at&sort_order=-1&category_id=%s", iteration, category_ids[flip_store_category_index]);
        return flipper_http_get_request_with_headers(fhttp, url, "{\"Content-Type\":\"application/json\"}");
    }
    bool parse_app_list()
    {
        return set_appropriate_list(fhttp, app);
    }
    flipper_http_loading_task(fhttp, fetch_app_list, parse_app_list, FlipStoreViewAppListCategory, FlipStoreViewSubmenuOptions, &app->view_dispatcher);
    flipper_http_free(fhttp);
}

void callback_submenu_choices(void *context, uint32_t index)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    switch (index)
    {
    case FlipStoreSubmenuIndexAbout:
        free_all_views(app, true);
        if (!alloc_about_view(app))
        {
            FURI_LOG_E(TAG, "Failed to set about view");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAbout);
        break;
    case FlipStoreSubmenuIndexSettings:
        free_all_views(app, true);
        if (!alloc_variable_item_list(app))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
        break;
    case FlipStoreSubmenuIndexOptions:
        free_all_views(app, true);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenuOptions);
        break;
    case FlipStoreSubmenuIndexAppList:
        flip_store_category_index = 0;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
        break;
    case FlipStoreSubmenuIndexFirmwares: // esp32 firmwares
        firmwares = firmware_alloc();
        if (firmwares == NULL)
        {
            FURI_LOG_E(TAG, "Failed to allocate memory for firmwares");
            return;
        }
        submenu_reset(app->submenu_firmwares);
        submenu_set_header(app->submenu_firmwares, "ESP32 Firmwares");
        for (int i = 0; i < FIRMWARE_COUNT; i++)
        {
            submenu_add_item(app->submenu_firmwares, firmwares[i].name, FlipStoreSubmenuIndexStartFirmwares + i, callback_submenu_choices, app);
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwares);
        break;
    case FlipStoreSubmenuIndexVGMFirmwares: // vgm firmwares
        vgm_firmwares = vgm_firmware_alloc();
        if (vgm_firmwares == NULL)
        {
            FURI_LOG_E(TAG, "Failed to allocate memory for vgm firmwares");
            return;
        }
        submenu_reset(app->submenu_vgm_firmwares);
        submenu_set_header(app->submenu_vgm_firmwares, "VGM Firmwares");
        for (int i = 0; i < VGM_FIRMWARE_COUNT; i++)
        {
            submenu_add_item(app->submenu_vgm_firmwares, vgm_firmwares[i].name, FlipStoreSubmenuIndexStartVGMFirmwares + i, callback_submenu_choices, app);
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewVGMFirmwares);
        break;
    case FlipStoreSubmenuIndexGitHub: // github
        free_all_views(app, true);
        if (!alloc_text_input_view(app, "Author"))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input view");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInput);
        break;
    case FlipStoreSubmenuIndexAppListBluetooth:
        free_all_views(app, true);
        flip_store_category_index = 0;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListGames:
        free_all_views(app, true);
        flip_store_category_index = 1;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListGPIO:
        free_all_views(app, true);
        flip_store_category_index = 2;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListInfrared:
        free_all_views(app, true);
        flip_store_category_index = 3;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListiButton:
        free_all_views(app, true);
        flip_store_category_index = 4;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListMedia:
        free_all_views(app, true);
        flip_store_category_index = 5;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListNFC:
        free_all_views(app, true);
        flip_store_category_index = 6;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListRFID:
        free_all_views(app, true);
        flip_store_category_index = 7;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListSubGHz:
        free_all_views(app, true);
        flip_store_category_index = 8;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListTools:
        free_all_views(app, true);
        flip_store_category_index = 9;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    case FlipStoreSubmenuIndexAppListUSB:
        free_all_views(app, true);
        flip_store_category_index = 10;
        flip_store_app_does_exist = false;
        fetch_appropiate_app_list(app, 0);
        break;
    default:
        // Check if the index is within the ESP32 firmwares list range
        if (index >= FlipStoreSubmenuIndexStartFirmwares && index < FlipStoreSubmenuIndexStartFirmwares + FIRMWARE_COUNT)
        {
            // Get the firmware index
            uint32_t firmware_index = index - FlipStoreSubmenuIndexStartFirmwares;

            // Check if the firmware index is valid
            if ((int)firmware_index >= 0 && firmware_index < FIRMWARE_COUNT)
            {
                // Get the firmware name
                selected_firmware_index = firmware_index;
                is_esp32_firmware = true;

                // Switch to the firmware download view
                free_dialog_firmware(app);
                if (!alloc_dialog_firmware(app))
                {
                    FURI_LOG_E(TAG, "Failed to allocate dialog firmware");
                    return;
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwareDialog);
            }
            else
            {
                FURI_LOG_E(TAG, "Invalid firmware index");
                easy_flipper_dialog("Error", "Issue parsing firmware.");
            }
        }
        // Check if the index is within the VGM firmwares list range
        else if (index >= FlipStoreSubmenuIndexStartVGMFirmwares && index < FlipStoreSubmenuIndexStartVGMFirmwares + VGM_FIRMWARE_COUNT)
        {
            // Get the firmware index
            uint32_t firmware_index = index - FlipStoreSubmenuIndexStartVGMFirmwares;

            // Check if the firmware index is valid
            if ((int)firmware_index >= 0 && firmware_index < VGM_FIRMWARE_COUNT)
            {
                // Get the firmware name
                selected_firmware_index = firmware_index;
                is_esp32_firmware = false;

                // Switch to the firmware download view
                free_dialog_firmware(app);
                if (!alloc_dialog_firmware(app))
                {
                    FURI_LOG_E(TAG, "Failed to allocate dialog firmware");
                    return;
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwareDialog);
            }
            else
            {
                FURI_LOG_E(TAG, "Invalid firmware index");
                easy_flipper_dialog("Error", "Issue parsing firmware.");
            }
        }
        // Check if the index is within the app list range
        else if (index >= FlipStoreSubmenuIndexStartAppList && index < FlipStoreSubmenuIndexStartAppList + flip_catalog->count)
        {
            // Get the app index
            uint32_t app_index = index - FlipStoreSubmenuIndexStartAppList;

            // Check if the app index is valid
            if ((int)app_index >= 0 && app_index < flip_catalog->count)
            {
                // Get the app name
                char *app_name = flip_catalog[app_index].app_name;

                // Check if the app name is valid
                if (app_name != NULL && strlen(app_name) > 0)
                {
                    app_selected_index = app_index;
                    flip_store_app_does_exist = app_exists(flip_catalog[app_selected_index].app_id, categories[flip_store_category_index]);
                    free_app_info_view(app);
                    if (!alloc_app_info_view(app))
                    {
                        FURI_LOG_E(TAG, "Failed to allocate app info view");
                        return;
                    }
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
        // Check if the index is for loading the next set of apps
        else if (index == FlipStoreSubmenuIndexStartAppList + flip_catalog->count)
        {
            catalog_iteration = flip_catalog->iteration + 8;
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewWidgetResult);
            free_category_submenu(app);
            flip_catalog_free();
            fetch_appropiate_app_list(app, catalog_iteration);
        }
        else
        {
            FURI_LOG_E(TAG, "Unknown submenu index");
        }
        break;
    }
}

static void settings_item_selected(void *context, uint32_t index)
{
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }
    char ssid[64];
    char pass[64];
    switch (index)
    {
    case 0: // Input SSID
        // Text Input
        free_all_views(app, false);
        if (!alloc_text_input_view(app, "SSID"))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input view");
            return;
        }
        // load SSID
        if (load_settings(ssid, sizeof(ssid), pass, sizeof(pass)))
        {
            strncpy(app->uart_text_input_temp_buffer, ssid, app->uart_text_input_buffer_size - 1);
            app->uart_text_input_temp_buffer[app->uart_text_input_buffer_size - 1] = '\0';
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInput);
        break;
    case 1: // Input Password
        free_all_views(app, false);
        if (!alloc_text_input_view(app, "Password"))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input view");
            return;
        }
        // load password
        if (load_settings(ssid, sizeof(ssid), pass, sizeof(pass)))
        {
            strncpy(app->uart_text_input_temp_buffer, pass, app->uart_text_input_buffer_size - 1);
            app->uart_text_input_temp_buffer[app->uart_text_input_buffer_size - 1] = '\0';
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewTextInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

static void flip_store_widget_set_text(char *message, Widget **widget)
{
    if (widget == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_set_widget_text - widget is NULL");
        DEV_CRASH();
        return;
    }
    if (message == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_set_widget_text - message is NULL");
        DEV_CRASH();
        return;
    }
    widget_reset(*widget);

    uint32_t message_length = strlen(message); // Length of the message
    uint32_t i = 0;                            // Index tracker
    uint32_t formatted_index = 0;              // Tracker for where we are in the formatted message
    char *formatted_message;                   // Buffer to hold the final formatted message

    // Allocate buffer with double the message length plus one for safety
    if (!easy_flipper_set_buffer(&formatted_message, message_length * 2 + 1))
    {
        return;
    }

    while (i < message_length)
    {
        uint32_t max_line_length = 31;                  // Maximum characters per line
        uint32_t remaining_length = message_length - i; // Remaining characters
        uint32_t line_length = (remaining_length < max_line_length) ? remaining_length : max_line_length;

        // Check for newline character within the current segment
        uint32_t newline_pos = i;
        bool found_newline = false;
        for (; newline_pos < i + line_length && newline_pos < message_length; newline_pos++)
        {
            if (message[newline_pos] == '\n')
            {
                found_newline = true;
                break;
            }
        }

        if (found_newline)
        {
            // If newline found, set line_length up to the newline
            line_length = newline_pos - i;
        }

        // Temporary buffer to hold the current line
        char line[32];
        strncpy(line, message + i, line_length);
        line[line_length] = '\0';

        // If newline was found, skip it for the next iteration
        if (found_newline)
        {
            i += line_length + 1; // +1 to skip the '\n' character
        }
        else
        {
            // Check if the line ends in the middle of a word and adjust accordingly
            if (line_length == max_line_length && message[i + line_length] != '\0' && message[i + line_length] != ' ')
            {
                // Find the last space within the current line to avoid breaking a word
                char *last_space = strrchr(line, ' ');
                if (last_space != NULL)
                {
                    // Adjust the line_length to avoid cutting the word
                    line_length = last_space - line;
                    line[line_length] = '\0'; // Null-terminate at the space
                }
            }

            // Move the index forward by the determined line_length
            i += line_length;

            // Skip any spaces at the beginning of the next line
            while (i < message_length && message[i] == ' ')
            {
                i++;
            }
        }

        // Manually copy the fixed line into the formatted_message buffer
        for (uint32_t j = 0; j < line_length; j++)
        {
            formatted_message[formatted_index++] = line[j];
        }

        // Add a newline character for line spacing
        formatted_message[formatted_index++] = '\n';
    }

    // Null-terminate the formatted_message
    formatted_message[formatted_index] = '\0';

    // Add the formatted message to the widget
    widget_add_text_scroll_element(*widget, 0, 0, 128, 64, formatted_message);
}
static void flip_store_request_error(Canvas *canvas, FlipperHTTP *fhttp)
{
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return;
    }
    if (fhttp->last_response != NULL)
    {
        if (strstr(fhttp->last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(fhttp->last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else
        {
            FURI_LOG_E(TAG, "Received an error: %s", fhttp->last_response);
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

void flip_store_loader_draw_callback(Canvas *canvas, void *model)
{
    if (!canvas || !model)
    {
        FURI_LOG_E(TAG, "flip_store_loader_draw_callback - canvas or model is NULL");
        return;
    }

    DataLoaderModel *data_loader_model = (DataLoaderModel *)model;
    SerialState http_state = data_loader_model->fhttp->state;
    DataState data_state = data_loader_model->data_state;
    char *title = data_loader_model->title;

    canvas_set_font(canvas, FontSecondary);

    if (http_state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    if (data_state == DataStateError || data_state == DataStateParseError)
    {
        flip_store_request_error(canvas, data_loader_model->fhttp);
        return;
    }

    canvas_draw_str(canvas, 0, 7, title);
    canvas_draw_str(canvas, 0, 17, "Loading...");

    if (data_state == DataStateInitial)
    {
        return;
    }

    if (http_state == SENDING)
    {
        canvas_draw_str(canvas, 0, 27, "Fetching...");
        return;
    }

    if (http_state == RECEIVING || data_state == DataStateRequested)
    {
        canvas_draw_str(canvas, 0, 27, "Receiving...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateReceived)
    {
        canvas_draw_str(canvas, 0, 27, "Processing...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateParsed)
    {
        canvas_draw_str(canvas, 0, 27, "Processed...");
        return;
    }
}

static void flip_store_loader_process_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_process_callback - context is NULL");
        DEV_CRASH();
        return;
    }

    FlipStoreApp *app = (FlipStoreApp *)context;
    View *view = app->view_loader;

    DataState current_data_state;
    DataLoaderModel *loader_model = NULL;
    with_view_model(view, DataLoaderModel * model, { current_data_state = model->data_state; loader_model = model; }, false);
    if (!loader_model || !loader_model->fhttp)
    {
        FURI_LOG_E(TAG, "Model or fhttp is NULL");
        DEV_CRASH();
        return;
    }

    if (current_data_state == DataStateInitial)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                model->data_state = DataStateRequested;
                DataLoaderFetch fetch = model->fetcher;
                if (fetch == NULL)
                {
                    FURI_LOG_E(TAG, "Model doesn't have Fetch function assigned.");
                    model->data_state = DataStateError;
                    return;
                }

                // Clear any previous responses
                strncpy(model->fhttp->last_response, "", 1);
                bool request_status = fetch(model);
                if (!request_status)
                {
                    model->data_state = DataStateError;
                }
            },
            true);
    }
    else if (current_data_state == DataStateRequested || current_data_state == DataStateError)
    {
        if (loader_model->fhttp->state == IDLE && loader_model->fhttp->last_response != NULL)
        {
            if (strstr(loader_model->fhttp->last_response, "[PONG]") != NULL)
            {
                FURI_LOG_DEV(TAG, "PONG received.");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[SUCCESS]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "SUCCESS received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[ERROR]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "ERROR received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strlen(loader_model->fhttp->last_response) == 0)
            {
                // Still waiting on response
            }
            else
            {
                with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateReceived; }, true);
            }
        }
        else if (loader_model->fhttp->state == SENDING || loader_model->fhttp->state == RECEIVING)
        {
            // continue waiting
        }
        else if (loader_model->fhttp->state == INACTIVE)
        {
            // inactive. try again
        }
        else if (loader_model->fhttp->state == ISSUE)
        {
            with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateError; }, true);
        }
        else
        {
            FURI_LOG_DEV(TAG, "Unexpected state: %d lastresp: %s", loader_model->fhttp->state, loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            DEV_CRASH();
        }
    }
    else if (current_data_state == DataStateReceived)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                char *data_text;
                if (model->parser == NULL)
                {
                    data_text = NULL;
                    FURI_LOG_DEV(TAG, "Parser is NULL");
                    DEV_CRASH();
                }
                else
                {
                    data_text = model->parser(model);
                }
                FURI_LOG_DEV(TAG, "Parsed data: %s\r\ntext: %s", model->fhttp->last_response ? model->fhttp->last_response : "NULL", data_text ? data_text : "NULL");
                model->data_text = data_text;
                if (data_text == NULL)
                {
                    model->data_state = DataStateParseError;
                }
                else
                {
                    model->data_state = DataStateParsed;
                }
            },
            true);
    }
    else if (current_data_state == DataStateParsed)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                if (++model->request_index < model->request_count)
                {
                    model->data_state = DataStateInitial;
                }
                else
                {
                    flip_store_widget_set_text(model->data_text != NULL ? model->data_text : "", &app->widget_result);
                    if (model->data_text != NULL)
                    {
                        free(model->data_text);
                        model->data_text = NULL;
                    }
                    view_set_previous_callback(widget_get_view(app->widget_result), model->back_callback);
                    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewWidgetResult);
                }
            },
            true);
    }
}

static void flip_store_loader_timer_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_timer_callback - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipStoreApp *app = (FlipStoreApp *)context;
    view_dispatcher_send_custom_event(app->view_dispatcher, FlipStoreCustomEventProcess);
}

static void flip_store_loader_on_enter(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_on_enter - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipStoreApp *app = (FlipStoreApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            view_set_previous_callback(view, model->back_callback);
            if (model->timer == NULL)
            {
                model->timer = furi_timer_alloc(flip_store_loader_timer_callback, FuriTimerTypePeriodic, app);
            }
            furi_timer_start(model->timer, 250);
        },
        true);
}

static void flip_store_loader_on_exit(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_on_exit - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipStoreApp *app = (FlipStoreApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_stop(model->timer);
            }
        },
        false);
}

void flip_store_loader_init(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_init - view is NULL");
        DEV_CRASH();
        return;
    }
    view_allocate_model(view, ViewModelTypeLocking, sizeof(DataLoaderModel));
    view_set_enter_callback(view, flip_store_loader_on_enter);
    view_set_exit_callback(view, flip_store_loader_on_exit);
}

void flip_store_loader_free_model(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_loader_free_model - view is NULL");
        DEV_CRASH();
        return;
    }
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_free(model->timer);
                model->timer = NULL;
            }
            if (model->parser_context)
            {
                // do not free the context here, it is the app context
                // free(model->parser_context);
                // model->parser_context = NULL;
            }
            if (model->fhttp)
            {
                flipper_http_free(model->fhttp);
                model->fhttp = NULL;
            }
        },
        false);
}

bool flip_store_custom_event_callback(void *context, uint32_t index)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_custom_event_callback - context is NULL");
        DEV_CRASH();
        return false;
    }

    switch (index)
    {
    case FlipStoreCustomEventProcess:
        flip_store_loader_process_callback(context);
        return true;
    default:
        FURI_LOG_DEV(TAG, "flip_store_custom_event_callback. Unknown index: %ld", index);
        return false;
    }
}

void flip_store_generic_switch_to_view(FlipStoreApp *app, char *title, DataLoaderFetch fetcher, DataLoaderParser parser, size_t request_count, ViewNavigationCallback back, uint32_t view_id)
{
    if (app == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_generic_switch_to_view - app is NULL");
        DEV_CRASH();
        return;
    }

    View *view = app->view_loader;
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "flip_store_generic_switch_to_view - view is NULL");
        DEV_CRASH();
        return;
    }

    with_view_model(
        view,
        DataLoaderModel * model,
        {
            model->title = title;
            model->fetcher = fetcher;
            model->parser = parser;
            model->request_index = 0;
            model->request_count = request_count;
            model->back_callback = back;
            model->data_state = DataStateInitial;
            model->data_text = NULL;
            //
            model->parser_context = app;
            if (!model->fhttp)
            {
                model->fhttp = flipper_http_alloc();
            }
        },
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, view_id);
}
