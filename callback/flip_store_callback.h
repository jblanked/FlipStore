#ifndef FLIP_STORE_CALLBACK_H
#define FLIP_STORE_CALLBACK_H
#include <flip_store.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <apps/flip_store_apps.h>
#include <flip_storage/flip_store_storage.h>

// Callback for drawing the main screen
void flip_store_view_draw_callback_main(Canvas *canvas, void *model);

void flip_store_view_draw_callback_app_list(Canvas *canvas, void *model);

bool flip_store_input_callback(InputEvent *event, void *context);

void flip_store_text_updated_ssid(void *context);

void flip_store_text_updated_pass(void *context);

uint32_t callback_to_submenu(void *context);

uint32_t callback_to_app_list(void *context);

void settings_item_selected(void *context, uint32_t index);

void dialog_callback(DialogExResult result, void *context);

void popup_callback(void *context);
/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
uint32_t callback_exit_app(void *context);
void callback_submenu_choices(void *context, uint32_t index);

#endif // FLIP_STORE_CALLBACK_H
