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
    if (!fhttp || !filename || !author || !repo || !link)
    {
        FURI_LOG_E(TAG, "Invalid arguments.");
        return false;
    }

    snprintf(fhttp->file_path, sizeof(fhttp->file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s/%s/%s.txt", author, repo, filename);
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

    // example path: /ext/apps_data/flip_store/author
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s", author);
    storage_common_mkdir(storage, dir);

    // example path: /ext/apps_data/flip_store/author/repo
    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s/%s", author, repo);
    storage_common_mkdir(storage, dir);

    furi_record_close(RECORD_STORAGE);

    // get the contents of the repo
    char link[256];
    snprintf(link, sizeof(link), "https://api.github.com/repos/%s/%s/contents", author, repo);
    fhttp->save_received_data = true;
    return flipper_http_get_request_with_headers(fhttp, link, "{\"Content-Type\":\"application/json\"}");
}

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Assuming necessary headers and definitions for FuriString, FURI_LOG, etc., are included.

bool flip_store_parse_github_contents(char *file_path, const char *author, const char *repo)
{
    FURI_LOG_I(TAG, "Parsing Github contents from %s - %s.", author, repo);
    if (!file_path || !author || !repo)
    {
        FURI_LOG_E(TAG, "Invalid arguments.");
        return false;
    }

    // Load JSON data from file
    FuriString *git_data = flipper_http_load_from_file(file_path);
    if (git_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Allocate a new FuriString to hold the entire JSON structure
    FuriString *git_data_str = furi_string_alloc();
    if (!git_data_str)
    {
        FURI_LOG_E(TAG, "Failed to allocate git_data_str.");
        furi_string_free(git_data);
        return false;
    }

    // Construct the full JSON string
    furi_string_cat_str(git_data_str, "{\"json_data\":");
    furi_string_cat(git_data_str, git_data);
    furi_string_cat_str(git_data_str, "}");
    furi_string_free(git_data); // Free the original git_data as it's now part of git_data_str

    // Check available memory
    const size_t additional_bytes = strlen("{\"json_data\":") + 1; // +1 for the closing "}"
    if (memmgr_get_free_heap() < furi_string_size(git_data_str) + additional_bytes)
    {
        FURI_LOG_E(TAG, "Not enough memory to allocate git_data_str.");
        furi_string_free(git_data_str);
        return false;
    }

    int file_count = 0;
    char dir[512]; // Increased size to accommodate longer paths if necessary
    FURI_LOG_I(TAG, "Looping through Github files.");
    FURI_LOG_I(TAG, "Available memory: %d bytes", memmgr_get_free_heap());

    // Get the C-string and its length for processing
    char *data = (char *)furi_string_get_cstr(git_data_str);
    size_t data_len = furi_string_size(git_data_str);

    size_t pos = 0; // Current position in the data string
    // Locate the start of the JSON array
    char *array_start = strchr(data, '[');
    if (!array_start)
    {
        FURI_LOG_E(TAG, "Invalid JSON format: '[' not found.");
        furi_string_free(git_data_str);
        return false;
    }
    pos = array_start - data; // Update position to the start of the array
    size_t brace_count = 0;
    size_t obj_start = 0;
    bool in_string = false; // To handle braces inside strings

    while (pos < data_len && file_count < MAX_GITHUB_FILES)
    {
        char current = data[pos];

        // Toggle in_string flag if a quote is found (handling escaped quotes)
        if (current == '"' && (pos == 0 || data[pos - 1] != '\\'))
        {
            in_string = !in_string;
        }

        if (!in_string)
        {
            if (current == '{')
            {
                if (brace_count == 0)
                {
                    obj_start = pos; // Potential start of a JSON object
                }
                brace_count++;
            }
            else if (current == '}')
            {
                brace_count--;
                if (brace_count == 0)
                {
                    size_t obj_end = pos;
                    size_t obj_length = obj_end - obj_start + 1;
                    // Extract the JSON object substring
                    char *obj_str = malloc(obj_length + 1);
                    if (!obj_str)
                    {
                        FURI_LOG_E(TAG, "Memory allocation failed for obj_str.");
                        break;
                    }
                    strncpy(obj_str, data + obj_start, obj_length);
                    obj_str[obj_length] = '\0'; // Null-terminate

                    FuriString *json_data_array = furi_string_alloc();
                    furi_string_set(json_data_array, obj_str); // Set the string to the allocated memory
                    free(obj_str);                             // Free the temporary C-string

                    if (!json_data_array)
                    {
                        FURI_LOG_E(TAG, "Failed to initialize json_data_array.");
                        break;
                    }

                    FURI_LOG_I(TAG, "Loaded json data array value %d. Available memory: %d bytes", file_count, memmgr_get_free_heap());

                    // Extract "type" field
                    FuriString *type = get_json_value_furi("type", json_data_array);
                    if (!type)
                    {
                        FURI_LOG_E(TAG, "Failed to get type.");
                        furi_string_free(json_data_array);
                        break;
                    }

                    // Skip non-file types (e.g., directories)
                    if (strcmp(furi_string_get_cstr(type), "file") != 0)
                    {
                        furi_string_free(type);
                        furi_string_free(json_data_array);
                        pos = obj_end + 1; // Move past this object
                        continue;
                    }
                    furi_string_free(type);

                    // Extract "download_url" and "name"
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
                    FURI_LOG_I(TAG, "Received name and download_url. Available memory: %d bytes", memmgr_get_free_heap());

                    // Create JSON to save
                    FuriString *json = furi_string_alloc();
                    if (!json)
                    {
                        FURI_LOG_E(TAG, "Failed to allocate json.");
                        furi_string_free(download_url);
                        furi_string_free(name);
                        break;
                    }

                    furi_string_cat_str(json, "{\"name\":\"");
                    furi_string_cat(json, name);
                    furi_string_cat_str(json, "\",\"link\":\"");
                    furi_string_cat(json, download_url);
                    furi_string_cat_str(json, "\"}");

                    FURI_LOG_I(TAG, "Created json. Available memory: %d bytes", memmgr_get_free_heap());

                    // Save the JSON to the data folder: /ext/apps_data/flip_store/data/author/repo/fileX.json
                    snprintf(dir, sizeof(dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/%s/file%d.json", author, repo, file_count);
                    if (!save_char_with_path(dir, furi_string_get_cstr(json)))
                    {
                        FURI_LOG_E(TAG, "Failed to save json.");
                    }

                    FURI_LOG_I(TAG, "Saved file %s.", furi_string_get_cstr(name));

                    // Free allocated resources
                    furi_string_free(name);
                    furi_string_free(download_url);
                    furi_string_free(json);

                    file_count++;

                    // This can be expensive for large strings; consider memory constraints
                    size_t remaining_length = data_len - (obj_end + 1);
                    memmove(data + obj_start, data + obj_end + 1, remaining_length + 1); // +1 to include null terminator
                    data_len -= (obj_end + 1 - obj_start);
                    pos = obj_start; // Reset position to the start of the modified string
                    continue;
                }
            }
        }

        pos++;
    }

    furi_string_free(git_data_str);

    // Save file count
    char file_count_str[16];
    snprintf(file_count_str, sizeof(file_count_str), "%d", file_count);
    char file_count_dir[512]; // Increased size for longer paths
    snprintf(file_count_dir, sizeof(file_count_dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/file_count.txt", author);

    FURI_LOG_I(TAG, "Successfully parsed %d files.", file_count);
    return save_char_with_path(file_count_dir, file_count_str);
}

bool flip_store_install_all_github_files(FlipperHTTP *fhttp, const char *author, const char *repo)
{
    FURI_LOG_I(TAG, "Installing all Github files.");
    if (!fhttp || !author || !repo)
    {
        FURI_LOG_E(TAG, "Invalid arguments.");
        return false;
    }
    fhttp->state = RECEIVING;
    // get the file count
    char file_count_dir[256]; // /ext/apps_data/flip_store/data/author/file_count.txt
    snprintf(file_count_dir, sizeof(file_count_dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/file_count.txt", author);
    FuriString *file_count = flipper_http_load_from_file(file_count_dir);
    if (file_count == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load file count.");
        return false;
    }
    int count = atoi(furi_string_get_cstr(file_count));
    furi_string_free(file_count);

    // install all files
    char file_dir[256]; // /ext/apps_data/flip_store/data/author/repo/file.json
    FURI_LOG_I(TAG, "Installing %d files.", count);
    for (int i = 0; i < count; i++)
    {
        snprintf(file_dir, sizeof(file_dir), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/data/%s/%s/file%d.json", author, repo, i);
        FURI_LOG_I(TAG, "Loading file %s. Available memory: %d bytes", file_dir, memmgr_get_free_heap());
        FuriString *file = flipper_http_load_from_file_with_limit(file_dir, 512);
        if (!file)
        {
            FURI_LOG_E(TAG, "Failed to load file.");
            return false;
        }
        FURI_LOG_I(TAG, "Loaded file %s.", file_dir);
        FuriString *name = get_json_value_furi("name", file);
        if (!name)
        {
            FURI_LOG_E(TAG, "Failed to get name.");
            furi_string_free(file);
            return false;
        }
        FuriString *link = get_json_value_furi("link", file);
        if (!link)
        {
            FURI_LOG_E(TAG, "Failed to get link.");
            furi_string_free(file);
            furi_string_free(name);
            return false;
        }
        furi_string_free(file);

        bool fetch_file()
        {
            return flip_store_download_github_file(fhttp, furi_string_get_cstr(name), author, repo, furi_string_get_cstr(link));
        }

        bool parse()
        {
            // remove .txt from the filename
            char current_file_path[512];
            char new_file_path[512];
            snprintf(current_file_path, sizeof(current_file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s/%s/%s.txt", author, repo, furi_string_get_cstr(name));
            snprintf(new_file_path, sizeof(new_file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_store/%s/%s/%s", author, repo, furi_string_get_cstr(name));
            Storage *storage = furi_record_open(RECORD_STORAGE);
            if (!storage_file_exists(storage, current_file_path))
            {
                FURI_LOG_E(TAG, "Failed to download file.");
                furi_record_close(RECORD_STORAGE);
                return false;
            }
            if (storage_common_rename(storage, current_file_path, new_file_path) != FSE_OK)
            {
                FURI_LOG_E(TAG, "Failed to rename file.");
                furi_record_close(RECORD_STORAGE);
                return false;
            }
            furi_record_close(RECORD_STORAGE);
            return true;
        }
        // download the file and wait until it is downloaded
        FURI_LOG_I(TAG, "Downloading file %s", furi_string_get_cstr(name));
        if (!flipper_http_process_response_async(fhttp, fetch_file, parse))
        {
            FURI_LOG_E(TAG, "Failed to download file.");
            furi_string_free(name);
            furi_string_free(link);
            return false;
        }
        FURI_LOG_I(TAG, "Downloaded file %s", furi_string_get_cstr(name));
        furi_string_free(name);
        furi_string_free(link);
    }
    fhttp->state = IDLE;
    return true;
}