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
#include <jsmn/jsmn_furi.h>
#include <flip_store_icons.h>

#define TAG "FlipStore"
#define VERSION_TAG "FlipStore v0.8"

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
    FlipStoreViewTextInput,       // The text input screen
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
    DialogEx *dialog_firmware; // The dialog for installing a firmware
    //
    Submenu *submenu_main; // The submenu (main)
    //
    Submenu *submenu_options;             // The submenu (options)
    Submenu *submenu_app_list;            // The submenu (app list) for the selected category
    Submenu *submenu_firmwares;           // The submenu (firmwares)
    Submenu *submenu_app_list_category;   // The submenu (app list) for each category
    Widget *widget_about;                 // The widget
    VariableItemList *variable_item_list; // The variable item list (settngs)
    VariableItem *variable_item_ssid;     // The variable item
    VariableItem *variable_item_pass;     // The variable item
    //
    UART_TextInput *uart_text_input;      // The text input
    char *uart_text_input_buffer;         // Buffer for the text input
    char *uart_text_input_temp_buffer;    // Temporary buffer for the text input
    uint32_t uart_text_input_buffer_size; // Size of the text input buffer
} FlipStoreApp;

void flip_store_app_free(FlipStoreApp *app);

#endif // FLIP_STORE_E_H