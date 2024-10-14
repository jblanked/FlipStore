#ifndef FLIP_STORE_E_H
#define FLIP_STORE_E_H
#include <uart_text_input.h>
#include <flipper_http.h>
#include <easy_flipper.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/modules/submenu.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>
#include <dialogs/dialogs.h>
#define TAG "FlipStore"

// Define the submenu items for our FlipStore application
typedef enum
{
    FlipStoreSubmenuIndexMain, // Click to start downloading the selected app
    FlipStoreSubmenuIndexAbout,
    FlipStoreSubmenuIndexSettings,
    FlipStoreSubmenuIndexAppList,
    FlipStoreSubmenuIndexDownloadApp,
    //
    FlipStoreSubmenuIndexStartAppList
} FlipStoreSubmenuIndex;

// Define a single view for our FlipStore application
typedef enum
{
    FlipStoreViewMain,          // The main screen
    FlipStoreViewSubmenu,       // The submenu
    FlipStoreViewAbout,         // The about screen
    FlipStoreViewSettings,      // The settings screen
    FlipStoreViewTextInputSSID, // The text input screen for SSID
    FlipStoreViewTextInputPass, // The text input screen for password
    //
    FlipStoreViewPopup, // The popup screen
    //
    FlipStoreViewAppList,     // The app list screen
    FlipStoreViewAppInfo,     // The app info screen (widget) of the selected app
    FlipStoreViewAppDownload, // The app download screen (widget) of the selected app
    FlipStoreViewAppDelete,   // The app delete screen (DialogEx) of the selected app
    //
} FlipStoreView;

// Each screen will have its own view
typedef struct
{
    ViewDispatcher *view_dispatcher;      // Switches between our views
    View *view_main;                      // The main screen for downloading apps
    View *view_app_info;                  // The app info screen (view) of the selected app
    Submenu *submenu;                     // The submenu (main)
    Submenu *submenu_app_list;            // The submenu (app list)
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
    uint32_t uart_text_input_buffer_size_pass; // Size of
} FlipStoreApp;

#endif // FLIP_STORE_E_H