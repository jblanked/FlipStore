#ifndef FLIP_STORE_CALLBACK_H
#define FLIP_STORE_CALLBACK_H
#include <flip_store.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <apps/flip_store_apps.h>
#include <firmwares/flip_store_firmwares.h>
#include <flip_storage/flip_store_storage.h>

#define MAX_LINE_LENGTH 30

extern bool flip_store_app_does_exist;
extern uint32_t selected_firmware_index;

uint32_t callback_to_submenu(void *context);

uint32_t callback_to_submenu_options(void *context);

uint32_t callback_to_firmware_list(void *context);
uint32_t callback_to_vgm_firmware_list(void *context);

uint32_t callback_to_app_list(void *context);

uint32_t callback_exit_app(void *context);
void callback_submenu_choices(void *context, uint32_t index);

void free_all_views(FlipStoreApp *app, bool should_free_variable_item_list);

// Add edits by Derek Jamison
typedef enum DataState DataState;
enum DataState
{
    DataStateInitial,
    DataStateRequested,
    DataStateReceived,
    DataStateParsed,
    DataStateParseError,
    DataStateError,
};

typedef enum FlipStoreCustomEvent FlipStoreCustomEvent;
enum FlipStoreCustomEvent
{
    FlipStoreCustomEventProcess,
};

typedef struct DataLoaderModel DataLoaderModel;
typedef bool (*DataLoaderFetch)(DataLoaderModel *model);
typedef char *(*DataLoaderParser)(DataLoaderModel *model);
struct DataLoaderModel
{
    char *title;
    char *data_text;
    DataState data_state;
    DataLoaderFetch fetcher;
    DataLoaderParser parser;
    void *parser_context;
    size_t request_index;
    size_t request_count;
    ViewNavigationCallback back_callback;
    FuriTimer *timer;
    FlipperHTTP *fhttp;
};
void flip_store_generic_switch_to_view(FlipStoreApp *app, char *title, DataLoaderFetch fetcher, DataLoaderParser parser, size_t request_count, ViewNavigationCallback back, uint32_t view_id);

void flip_store_loader_draw_callback(Canvas *canvas, void *model);

void flip_store_loader_init(View *view);

void flip_store_loader_free_model(View *view);

bool flip_store_custom_event_callback(void *context, uint32_t index);

#endif // FLIP_STORE_CALLBACK_H
