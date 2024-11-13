#include <callback/flip_store_callback.h>

bool flip_store_app_does_exist = false;
uint32_t selected_firmware_index = 0;

// Callback for drawing the main screen
void flip_store_view_draw_callback_main(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_set_font(canvas, FontSecondary);

    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    if (!flip_store_sent_request)
    {
        flip_store_sent_request = true;

        if (!flip_store_install_app(canvas, categories[flip_store_category_index]))
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Failed to install app.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
    }
    else
    {
        if (flip_store_success)
        {
            if (fhttp.state == RECEIVING)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "Downloading app...");
                canvas_draw_str(canvas, 0, 60, "Please wait...");
                return;
            }
            else if (fhttp.state == IDLE)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "App installed successfully.");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
        }
        else
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Failed to install app.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
    }
}

// Function to draw the firmware download screen
void flip_store_view_draw_callback_firmware(Canvas *canvas, void *model)
{
    UNUSED(model);

    // Check if the HTTP state is inactive
    if (fhttp.state == INACTIVE)
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    // Set font and clear the canvas for the loading state
    canvas_set_font(canvas, FontSecondary);
    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Loading...");

    // Handle first firmware file
    if (!sent_firmware_request)
    {
        sent_firmware_request = true;
        firmware_request_success = flip_store_get_firmware_file(
            firmwares[selected_firmware_index].links[0],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[0], '/') + 1);

        if (!firmware_request_success)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        return;
    }
    else if (sent_firmware_request && !firmware_download_success)
    {
        if (!firmware_request_success || fhttp.state == ISSUE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        else if (fhttp.state == RECEIVING)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Downloading file 1...");
            canvas_draw_str(canvas, 0, 60, "Please wait...");
        }
        else if (fhttp.state == IDLE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Success");
            canvas_draw_str(canvas, 0, 60, "Downloading the next file now.");
            firmware_download_success = true;
        }
        return;
    }

    // Handle second firmware file
    if (firmware_download_success && !sent_firmware_request_2)
    {
        sent_firmware_request_2 = true;
        firmware_request_success_2 = flip_store_get_firmware_file(
            firmwares[selected_firmware_index].links[1],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[1], '/') + 1);

        if (!firmware_request_success_2)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        return;
    }
    else if (sent_firmware_request_2 && !firmware_download_success_2)
    {
        if (!firmware_request_success_2 || fhttp.state == ISSUE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        else if (fhttp.state == RECEIVING)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Downloading file 2...");
            canvas_draw_str(canvas, 0, 60, "Please wait...");
        }
        else if (fhttp.state == IDLE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Success");
            canvas_draw_str(canvas, 0, 60, "Downloading the next file now.");
            firmware_download_success_2 = true;
        }
        return;
    }

    // Handle third firmware file
    if (firmware_download_success && firmware_download_success_2 && !sent_firmware_request_3)
    {
        sent_firmware_request_3 = true;
        firmware_request_success_3 = flip_store_get_firmware_file(
            firmwares[selected_firmware_index].links[2],
            firmwares[selected_firmware_index].name,
            strrchr(firmwares[selected_firmware_index].links[2], '/') + 1);

        if (!firmware_request_success_3)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        return;
    }
    else if (sent_firmware_request_3 && !firmware_download_success_3)
    {
        if (!firmware_request_success_3 || fhttp.state == ISSUE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            flip_store_request_error(canvas);
        }
        else if (fhttp.state == RECEIVING)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Downloading file 3...");
            canvas_draw_str(canvas, 0, 60, "Please wait...");
        }
        else if (fhttp.state == IDLE)
        {
            canvas_set_font(canvas, FontSecondary);
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Success");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            firmware_download_success_3 = true;
        }
        return;
    }

    // All files downloaded successfully
    if (firmware_download_success && firmware_download_success_2 && firmware_download_success_3)
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_clear(canvas);
        canvas_draw_str(canvas, 0, 10, "Files downloaded successfully.");
        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
    }
}

// Function to draw the message on the canvas with word wrapping
void draw_description(Canvas *canvas, const char *description, int x, int y)
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

void flip_store_view_draw_callback_app_list(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    char title[30];
    snprintf(title, 30, "%s (v.%s)", flip_catalog[app_selected_index].app_name, flip_catalog[app_selected_index].app_version);
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

bool flip_store_input_callback(InputEvent *event, void *context)
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

void flip_store_text_updated_ssid(void *context)
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
void flip_store_text_updated_pass(void *context)
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

uint32_t callback_to_submenu(void *context)
{
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    firmware_free();
    return FlipStoreViewSubmenu;
}

uint32_t callback_to_submenu_options(void *context)
{
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    firmware_free();
    return FlipStoreViewSubmenuOptions;
}

uint32_t callback_to_firmware_list(void *context)
{
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    sent_firmware_request = false;
    sent_firmware_request_2 = false;
    sent_firmware_request_3 = false;
    //
    firmware_request_success = false;
    firmware_request_success_2 = false;
    firmware_request_success_3 = false;
    //
    firmware_download_success = false;
    firmware_download_success_2 = false;
    firmware_download_success_3 = false;
    return FlipStoreViewFirmwares;
}

uint32_t callback_to_app_list(void *context)
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
    flip_store_app_does_exist = false;
    sent_firmware_request = false;
    return FlipStoreViewAppList;
}

void settings_item_selected(void *context, uint32_t index)
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

void dialog_delete_callback(DialogExResult result, void *context)
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
        if (!delete_app(flip_catalog[app_selected_index].app_id, categories[flip_store_category_index]))
        {
            // pop up a message
            popup_set_header(app->popup, "[ERROR]", 0, 0, AlignLeft, AlignTop);
            popup_set_text(app->popup, "Issue deleting app.", 0, 50, AlignLeft, AlignTop);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
            furi_delay_ms(2000); // delay for 2 seconds
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
        }
        else
        {
            // pop up a message
            popup_set_header(app->popup, "[SUCCESS]", 0, 0, AlignLeft, AlignTop);
            popup_set_text(app->popup, "App deleted successfully.", 0, 50, AlignLeft, AlignTop);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
            furi_delay_ms(2000); // delay for 2 seconds
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
        }
    }
}

void dialog_firmware_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipStoreApp *app = (FlipStoreApp *)context;
    if (result == DialogExResultLeft) // No
    {
        // switch to the firmware list
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwares);
    }
    else if (result == DialogExResultRight)
    {
        // download the firmware then return to the firmware list
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwareDownload);
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

uint32_t callback_exit_app(void *context)
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
    case FlipStoreSubmenuIndexMain:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewMain);
        break;
    case FlipStoreSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAbout);
        break;
    case FlipStoreSubmenuIndexSettings:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSettings);
        break;
    case FlipStoreSubmenuIndexOptions:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenuOptions);
        break;
    case FlipStoreSubmenuIndexAppList:
        flip_store_category_index = 0;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewAppList);
        break;
    case FlipStoreSubmenuIndexFirmwares:
        if (!app->submenu_firmwares)
        {
            FURI_LOG_E(TAG, "Submenu firmwares is NULL");
            return;
        }
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
    case FlipStoreSubmenuIndexAppListBluetooth:
        flip_store_category_index = 0;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListBluetooth, "Bluetooth", &app->submenu_app_list_bluetooth));
        break;
    case FlipStoreSubmenuIndexAppListGames:
        flip_store_category_index = 1;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListGames, "Games", &app->submenu_app_list_games));
        break;
    case FlipStoreSubmenuIndexAppListGPIO:
        flip_store_category_index = 2;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListGPIO, "GPIO", &app->submenu_app_list_gpio));
        break;
    case FlipStoreSubmenuIndexAppListInfrared:
        flip_store_category_index = 3;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListInfrared, "Infrared", &app->submenu_app_list_infrared));
        break;
    case FlipStoreSubmenuIndexAppListiButton:
        flip_store_category_index = 4;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListiButton, "iButton", &app->submenu_app_list_ibutton));
        break;
    case FlipStoreSubmenuIndexAppListMedia:
        flip_store_category_index = 5;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListMedia, "Media", &app->submenu_app_list_media));
        break;
    case FlipStoreSubmenuIndexAppListNFC:
        flip_store_category_index = 6;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListNFC, "NFC", &app->submenu_app_list_nfc));
        break;
    case FlipStoreSubmenuIndexAppListRFID:
        flip_store_category_index = 7;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListRFID, "RFID", &app->submenu_app_list_rfid));
        break;
    case FlipStoreSubmenuIndexAppListSubGHz:
        flip_store_category_index = 8;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListSubGHz, "Sub-GHz", &app->submenu_app_list_subghz));
        break;
    case FlipStoreSubmenuIndexAppListTools:
        flip_store_category_index = 9;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListTools, "Tools", &app->submenu_app_list_tools));
        break;
    case FlipStoreSubmenuIndexAppListUSB:
        flip_store_category_index = 10;
        flip_store_app_does_exist = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, flip_store_handle_app_list(app, FlipStoreViewAppListUSB, "USB", &app->submenu_app_list_usb));
        break;
    default:
        // Check if the index is within the firmwares list range
        if (index >= FlipStoreSubmenuIndexStartFirmwares && index < FlipStoreSubmenuIndexStartFirmwares + 3)
        {
            // Get the firmware index
            uint32_t firmware_index = index - FlipStoreSubmenuIndexStartFirmwares;

            // Check if the firmware index is valid
            if ((int)firmware_index >= 0 && firmware_index < FIRMWARE_COUNT)
            {
                // Get the firmware name
                selected_firmware_index = firmware_index;

                // Switch to the firmware download view
                dialog_ex_set_header(app->dialog_firmware, firmwares[firmware_index].name, 0, 0, AlignLeft, AlignTop);
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewFirmwareDialog);
            }
            else
            {
                FURI_LOG_E(TAG, "Invalid firmware index");
                popup_set_header(app->popup, "[ERROR]", 0, 0, AlignLeft, AlignTop);
                popup_set_text(app->popup, "Issue parsing firmwarex", 0, 50, AlignLeft, AlignTop);
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewPopup);
            }
        }
        // Check if the index is within the app list range
        else if (index >= FlipStoreSubmenuIndexStartAppList && index < FlipStoreSubmenuIndexStartAppList + MAX_APP_COUNT)
        {
            // Get the app index
            uint32_t app_index = index - FlipStoreSubmenuIndexStartAppList;

            // Check if the app index is valid
            if ((int)app_index >= 0 && app_index < MAX_APP_COUNT)
            {
                // Get the app name
                char *app_name = flip_catalog[app_index].app_name;

                // Check if the app name is valid
                if (app_name != NULL && strlen(app_name) > 0)
                {
                    app_selected_index = app_index;
                    flip_store_app_does_exist = app_exists(flip_catalog[app_selected_index].app_id, categories[flip_store_category_index]);
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