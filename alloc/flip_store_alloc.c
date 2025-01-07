#include <alloc/flip_store_alloc.h>
// Function to allocate resources for the FlipStoreApp
FlipStoreApp *flip_store_app_alloc()
{
    FlipStoreApp *app = (FlipStoreApp *)malloc(sizeof(FlipStoreApp));

    Gui *gui = furi_record_open(RECORD_GUI);

    // Allocate ViewDispatcher
    if (!easy_flipper_set_view_dispatcher(&app->view_dispatcher, gui, app))
    {
        return NULL;
    }
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flip_store_custom_event_callback);

    // Main view
    if (!easy_flipper_set_view(&app->view_loader, FlipStoreViewLoader, flip_store_loader_draw_callback, NULL, callback_to_submenu_options, &app->view_dispatcher, app))
    {
        return NULL;
    }
    flip_store_loader_init(app->view_loader);

    if (!easy_flipper_set_widget(&app->widget_result, FlipStoreViewWidgetResult, "Error, try again.", callback_to_submenu_options, &app->view_dispatcher))
    {
        return NULL;
    }

    // Submenu
    if (!easy_flipper_set_submenu(&app->submenu_main, FlipStoreViewSubmenu, VERSION_TAG, callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_options, FlipStoreViewSubmenuOptions, "Browse", callback_to_submenu, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_app_list, FlipStoreViewAppList, "App Catalog", callback_to_submenu_options, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_firmwares, FlipStoreViewFirmwares, "ESP32 Firmware", callback_to_submenu_options, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_vgm_firmwares, FlipStoreViewVGMFirmwares, "VGM Firmware", callback_to_submenu_options, &app->view_dispatcher))
    {
        return NULL;
    }

    //
    submenu_add_item(app->submenu_main, "Browse", FlipStoreSubmenuIndexOptions, callback_submenu_choices, app);
    submenu_add_item(app->submenu_main, "About", FlipStoreSubmenuIndexAbout, callback_submenu_choices, app);
    submenu_add_item(app->submenu_main, "Settings", FlipStoreSubmenuIndexSettings, callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_options, "App Catalog", FlipStoreSubmenuIndexAppList, callback_submenu_choices, app);
    submenu_add_item(app->submenu_options, "ESP32 Firmware", FlipStoreSubmenuIndexFirmwares, callback_submenu_choices, app);
    submenu_add_item(app->submenu_options, "VGM Firmware", FlipStoreSubmenuIndexVGMFirmwares, callback_submenu_choices, app);
    submenu_add_item(app->submenu_options, "GitHub Repository", FlipStoreSubmenuIndexGitHub, callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_app_list, "Bluetooth", FlipStoreSubmenuIndexAppListBluetooth, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "Games", FlipStoreSubmenuIndexAppListGames, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "GPIO", FlipStoreSubmenuIndexAppListGPIO, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "Infrared", FlipStoreSubmenuIndexAppListInfrared, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "iButton", FlipStoreSubmenuIndexAppListiButton, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "Media", FlipStoreSubmenuIndexAppListMedia, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "NFC", FlipStoreSubmenuIndexAppListNFC, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "RFID", FlipStoreSubmenuIndexAppListRFID, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "Sub-GHz", FlipStoreSubmenuIndexAppListSubGHz, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "Tools", FlipStoreSubmenuIndexAppListTools, callback_submenu_choices, app);
    submenu_add_item(app->submenu_app_list, "USB", FlipStoreSubmenuIndexAppListUSB, callback_submenu_choices, app);
    //

    // Switch to the main view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipStoreViewSubmenu);

    return app;
}