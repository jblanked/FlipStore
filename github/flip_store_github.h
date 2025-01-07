#pragma once
#include <flip_store.h>

/*
I did try downloading a zip file from Github but a few issues
- unsure how to unzip the file
- either Github redirected us to a loading page, or their was no response on if using an IoT device
- any contributions to this would be appreciated
*/

// Helper to download a file from Github and save it to the storage
bool flip_store_download_github_file(
    FlipperHTTP *fhttp,
    const char *filename,
    const char *author,
    const char *repo,
    const char *link);

// Helper to get the contents of a Github repo (from https://api.github.com/repos/author/repo/contents)
bool flip_store_get_github_contents(FlipperHTTP *fhttp, const char *author, const char *repo);

// Helper to parse the contents of a Github repo (takes parts of the data and saves it for easy access later)
bool flip_store_parse_github_contents(char *file_path, const char *author, const char *repo);

// Helper to install all files from the parsed Github repo contents in flip_store_parse_github_contents
bool flip_store_install_all_github_files(FlipperHTTP *fhttp, const char *author, const char *repo);