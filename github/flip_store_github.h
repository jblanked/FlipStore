#pragma once
#include <flip_store.h>

// Helper to download a file from Github and save it to the storage
bool flip_store_download_github_file(
    FlipperHTTP *fhttp,
    const char *filename,
    const char *author,
    const char *repo,
    const char *link);

bool flip_store_get_github_contents(FlipperHTTP *fhttp, const char *author, const char *repo);
bool flip_store_parse_github_contents(char *file_path, const char *author, const char *repo);