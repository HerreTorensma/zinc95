#pragma once

#include "mem.h"
#include "string.h"

// Creates a directory in the zinc95 directory
void create_directory(string_t path);

// Returns a temporarily allocated array of strings names of directories in a path
string_t_array_t get_directories_in_path(allocator_t allocator, string_t path);

// Returns a temporarily allocated array of strings names of files in a path
string_t_array_t get_files_in_path(allocator_t allocator, string_t path);

// Create the default directories
// AppData/Roaming on Windows
void create_default_directories();

// Check if the path exists and is a directory
bool path_is_dir(string_t path);

// Check if the path exists and is a file
bool path_is_file(string_t path);
