#pragma once

#include "mem.h"
#include "string.h"

// TODO: fix segfault where you try to save a file but a directory of the same name already exists

string_t get_absolute_path(allocator_t allocator, string_t path);

// Creates a directory relative to the zinc95 directory
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

void file_write_string(string_t path, string_t string);

string_t file_load_to_string(allocator_t allocator, string_t path);
