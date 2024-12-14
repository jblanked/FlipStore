#ifndef FLIP_STORE_E_H
#define FLIP_STORE_E_H
#include <text_input/uart_text_input.h>
#include <flipper_http/flipper_http.h>
#include <easy_flipper/easy_flipper.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>
#include <dialogs/dialogs.h>
#include <jsmn/jsmn.h>
#include <flip_store_icons.h>
#define TAG "FlipStore"
#define FIRMWARE_COUNT 3
#define FIRMWARE_LINKS 3

// Define the submenu items for our FlipStore application
typedef enum
{
    FlipStoreSubmenuIndexMain, // Click to start downloading the selected app
    FlipStoreSubmenuIndexAbout,
    FlipStoreSubmenuIndexSettings,
    //
    FlipStoreSubmenuIndexOptions, // Click to view the options
    //
    FlipStoreSubmenuIndexAppList,
    FlipStoreSubmenuIndexFirmwares,
    //
    FlipStoreSubmenuIndexAppListBluetooth,
    FlipStoreSubmenuIndexAppListGames,
    FlipStoreSubmenuIndexAppListGPIO,
    FlipStoreSubmenuIndexAppListInfrared,
    FlipStoreSubmenuIndexAppListiButton,
    FlipStoreSubmenuIndexAppListMedia,
    FlipStoreSubmenuIndexAppListNFC,
    FlipStoreSubmenuIndexAppListRFID,
    FlipStoreSubmenuIndexAppListSubGHz,
    FlipStoreSubmenuIndexAppListTools,
    FlipStoreSubmenuIndexAppListUSB,
    //
    FlipStoreSubmenuIndexStartFirmwares,
    //
    FlipStoreSubmenuIndexStartAppList = 100,
} FlipStoreSubmenuIndex;

// Define a single view for our FlipStore application
typedef enum
{
    //
    FlipStoreViewSubmenu,         // The submenu
    FlipStoreViewSubmenuOptions,  // The submenu options
                                  //
    FlipStoreViewAbout,           // The about screen
    FlipStoreViewSettings,        // The settings screen
    FlipStoreViewTextInputSSID,   // The text input screen for SSID
    FlipStoreViewTextInputPass,   // The text input screen for password
                                  //
    FlipStoreViewPopup,           // The popup screen
                                  //
    FlipStoreViewAppList,         // The app list screen
    FlipStoreViewFirmwares,       // The firmwares screen (submenu)
    FlipStoreViewFirmwareDialog,  // The firmware view (DialogEx) of the selected firmware
                                  //
    FlipStoreViewAppInfo,         // The app info screen (widget) of the selected app
    FlipStoreViewAppDownload,     // The app download screen (widget) of the selected app
    FlipStoreViewAppDelete,       // The app delete screen (DialogEx) of the selected app
                                  //
    FlipStoreViewAppListCategory, // the app list screen for each category
                                  //
                                  //
    FlipStoreViewWidgetResult,    // The text box that displays the random fact
    FlipStoreViewLoader,          // The loader screen retrieves data from the internet
} FlipStoreView;

// Each screen will have its own view
typedef struct
{
    View *view_loader;
    Widget *widget_result;
    //
    ViewDispatcher *view_dispatcher; // Switches between our views
    View *view_app_info;             // The app info screen (view) of the selected app
    //
    DialogEx *dialog_firmware;    // The dialog for installing a firmware
    View *view_firmware_download; // The firmware download screen (view) of the selected firmware
    //
    Submenu *submenu_main; // The submenu (main)
    //
    Submenu *submenu_options;             // The submenu (options)
    Submenu *submenu_app_list;            // The submenu (app list) for the selected category
    Submenu *submenu_firmwares;           // The submenu (firmwares)
    Submenu *submenu_app_list_category;   // The submenu (app list) for each category
    Widget *widget;                       // The widget
    Popup *popup;                         // The popup
    DialogEx *dialog_delete;              // The dialog for deleting an app
    VariableItemList *variable_item_list; // The variable item list (settngs)
    VariableItem *variable_item_ssid;     // The variable item
    VariableItem *variable_item_pass;     // The variable item
    UART_TextInput *uart_text_input_ssid; // The text input
    UART_TextInput *uart_text_input_pass; // The text input

    char *uart_text_input_buffer_ssid;         // Buffer for the text input
    char *uart_text_input_temp_buffer_ssid;    // Temporary buffer for the text input
    uint32_t uart_text_input_buffer_size_ssid; // Size of the text input buffer

    char *uart_text_input_buffer_pass;         // Buffer for the text input
    char *uart_text_input_temp_buffer_pass;    // Temporary buffer for the text input
    uint32_t uart_text_input_buffer_size_pass; // Size of the text input buffer
} FlipStoreApp;

void flip_store_app_free(FlipStoreApp *app);

void flip_store_request_error(Canvas *canvas);
extern FlipStoreApp *app_instance;

#endif // FLIP_STORE_E_H