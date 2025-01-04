#include <flip_store.h>
#include <apps/flip_store_apps.h>

// Function to free the resources used by FlipStoreApp
void flip_store_app_free(FlipStoreApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipStoreApp is NULL");
        return;
    }

    // Free Widget(s)
    if (app->widget_result)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewWidgetResult);
        widget_free(app->widget_result);
    }

    // Free View(s)
    if (app->view_loader)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewLoader);
        flip_store_loader_free_model(app->view_loader);
        view_free(app->view_loader);
    }

    // Free Submenu(s)
    if (app->submenu_main)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewSubmenu);
        submenu_free(app->submenu_main);
    }
    if (app->submenu_options)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewSubmenuOptions);
        submenu_free(app->submenu_options);
    }
    if (app->submenu_app_list)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewAppList);
        submenu_free(app->submenu_app_list);
    }
    if (app->submenu_firmwares)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewFirmwares);
        submenu_free(app->submenu_firmwares);
    }
    if (app->submenu_vgm_firmwares)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipStoreViewVGMFirmwares);
        submenu_free(app->submenu_vgm_firmwares);
    }

    free_all_views(app, true);

    // free the view dispatcher
    if (app->view_dispatcher)
        view_dispatcher_free(app->view_dispatcher);

    // close the gui
    furi_record_close(RECORD_GUI);

    // free the app
    if (app)
        free(app);
}