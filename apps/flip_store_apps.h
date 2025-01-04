#ifndef FLIP_STORE_APPS_H
#define FLIP_STORE_APPS_H

#include <flip_store.h>
#include <flip_storage/flip_store_storage.h>
#include <callback/flip_store_callback.h>

// Define maximum limits
#define MAX_APP_NAME_LENGTH 32
#define MAX_ID_LENGTH 32
#define MAX_APP_COUNT 50
#define MAX_APP_DESCRIPTION_LENGTH 100
#define MAX_APP_VERSION_LENGTH 5

// define the list of categories
extern char *category_ids[];
extern char *categories[];
extern int catalog_iteration;

typedef struct
{
    char app_name[MAX_APP_NAME_LENGTH];
    char app_id[MAX_APP_NAME_LENGTH];
    char app_build_id[MAX_ID_LENGTH];
    char app_version[MAX_APP_VERSION_LENGTH];
    char app_description[MAX_APP_DESCRIPTION_LENGTH];
    size_t count;
    int iteration;
} FlipStoreAppInfo;

extern FlipStoreAppInfo *flip_catalog;
extern uint32_t app_selected_index;
extern uint32_t flip_store_category_index;

enum ObjectState
{
    OBJECT_EXPECT_KEY,
    OBJECT_EXPECT_COLON,
    OBJECT_EXPECT_VALUE,
    OBJECT_EXPECT_COMMA_OR_END
};

FlipStoreAppInfo *flip_catalog_alloc();

void flip_catalog_free();

// Utility function to parse JSON incrementally from a file
bool flip_store_process_app_list(FlipperHTTP *fhttp);

// function to handle the entire installation process "asynchronously"
bool flip_store_install_app(FlipperHTTP *fhttp, char *category);
#endif // FLIP_STORE_APPS_H