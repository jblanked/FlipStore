#include <github/flip_store_github.h>
#include <flip_storage/flip_store_storage.h>

// Helper to download a file from Github and save it to the storage
bool flip_store_download_github_file(
    FlipperHTTP *fhttp,
    const char *filename,
    const char *author,
    const char *repo,
    const char *link)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // Create the file directory
    char dir[256];
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s/%s/%s", author, repo, filename);
    snprintf(fhttp->file_path, sizeof(fhttp->file_path), "%s", dir);
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, dir);
    furi_record_close(RECORD_STORAGE);

    fhttp->state = IDLE;
    fhttp->save_received_data = false;
    fhttp->is_bytes_request = true;

    return flipper_http_get_request_bytes(fhttp, link, "{\"Content-Type\":\"application/octet-stream\"}");
}

bool flip_store_get_github_contents(FlipperHTTP *fhttp, const char *author, const char *repo)
{
    // Create Initial directory
    Storage *storage = furi_record_open(RECORD_STORAGE);

    char dir[256];

    // create a data directory: /ext/apps_data/flip_store/data
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data");
    storage_common_mkdir(storage, dir);

    // create a data directory for the author: /ext/apps_data/flip_store/data/author
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s", author);
    storage_common_mkdir(storage, dir);

    // example path: /ext/apps_data/flip_store/data/author/info.json
    snprintf(fhttp->file_path, sizeof(fhttp->file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/info.json", author);

    // create a data directory for the repo: /ext/apps_data/flip_store/data/author/repo
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/%s", author, repo);
    storage_common_mkdir(storage, dir);

    furi_record_close(RECORD_STORAGE);

    // get the contents of the repo
    char link[256];
    snprintf(link, sizeof(link), "https://api.github.com/repos/%s/%s/contents", author, repo);
    fhttp->save_received_data = true;
    return flipper_http_get_request_with_headers(fhttp, link, "{\"Content-Type\":\"application/json\"}");
}

bool flip_store_parse_github_contents(char *file_path, const char *author, const char *repo)
{
    // Parse list of files and prepare to download
    FuriString *git_data = flipper_http_load_from_file(file_path);
    if (git_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }
    FuriString *git_data_str = furi_string_alloc();
    if (!git_data_str)
    {
        FURI_LOG_E("Game", "Failed to allocate json_data string");
        return false;
    }
    furi_string_cat_str(git_data_str, "{\"json_data\":");
    if (memmgr_get_free_heap() < furi_string_size(git_data) + furi_string_size(git_data_str) + 2)
    {
        FURI_LOG_E(TAG, "Not enough memory to allocate git_data_str.");
        furi_string_free(git_data);
        furi_string_free(git_data_str);
        return false;
    }
    furi_string_cat(git_data_str, git_data);
    furi_string_free(git_data);
    furi_string_cat_str(git_data_str, "}");
    //
    int file_count = 0;
    for (int i = 0; i < MAX_GITHUB_FILES; i++)
    {
        FuriString *json_data_array = get_json_array_value_furi("json_data", i, git_data_str);
        if (!json_data_array)
        {
            break;
        }
        FuriString *type = get_json_value_furi("type", json_data_array);
        if (!type)
        {
            FURI_LOG_E(TAG, "Failed to get type.");
            furi_string_free(json_data_array);
            break;
        }
        // skip directories for now
        if (strcmp(furi_string_get_cstr(type), "file") != 0)
        {
            furi_string_free(type);
            furi_string_free(json_data_array);
            continue;
        }
        furi_string_free(type);
        FuriString *download_url = get_json_value_furi("download_url", json_data_array);
        if (!download_url)
        {
            FURI_LOG_E(TAG, "Failed to get download_url.");

            furi_string_free(json_data_array);
            break;
        }
        FuriString *name = get_json_value_furi("name", json_data_array);
        if (!name)
        {
            FURI_LOG_E(TAG, "Failed to get name.");

            furi_string_free(json_data_array);
            furi_string_free(download_url);
            break;
        }
        furi_string_free(json_data_array);

        // create json to save
        FuriString *json = furi_string_alloc();
        if (!json)
        {
            FURI_LOG_E(TAG, "Failed to allocate json.");

            furi_string_free(json_data_array);
            furi_string_free(download_url);
            furi_string_free(name);
            break;
        }

        furi_string_cat_str(json, "{\"name\":\"");
        furi_string_cat(json, name);

        furi_string_cat_str(json, "\",\"link\":\"");
        furi_string_cat(json, download_url);
        furi_string_free(download_url);

        furi_string_cat_str(json, "\"}");

        // save the json to the data folder: /ext/apps_data/flip_store/data/author/repo
        char dir[256];
        snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/%s/%s.json", author, repo, furi_string_get_cstr(name));
        if (!save_char_with_path(dir, furi_string_get_cstr(json)))
        {
            FURI_LOG_E(TAG, "Failed to save json.");
        }

        // free the json
        furi_string_free(name);
        furi_string_free(json);
        file_count++;
    }
    furi_string_free(git_data_str);
    // save file count
    char file_count_str[16];
    snprintf(file_count_str, sizeof(file_count_str), "%d", file_count);
    char file_count_dir[256]; // /ext/apps_data/flip_store/data/author/file_count.txt
    snprintf(file_count_dir, sizeof(file_count_dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/file_count.txt", author);
    return save_char_with_path(file_count_dir, file_count_str);
}