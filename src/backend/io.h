#pragma once

#include "mem.h"

// Creates a directory in the zinc95 directory
void create_directory(string_t path);

// Returns a temporarily allocated array of strings names of directories in a path
void get_directories_in_path(string_t path);

// Returns a temporarily allocated array of strings names of files in a path
void get_files_in_path(string_t path);

// Create the default directories
// AppData/Roaming on Windows
void create_default_directories();