#include <flip_store.h>
#include <apps/flip_store_apps.h>

FlipStoreApp *app_instance = NULL;

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

    // free the view dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // close the gui
    furi_record_close(RECORD_GUI);

    // free the app
    free(app);
}

void flip_store_request_error(Canvas *canvas)
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