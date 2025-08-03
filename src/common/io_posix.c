#include "io.h"
#include "mem.h"
#include "string.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static string_t _get_root_path(allocator_t allocator) {
	string_t root_path = {0};

	// Create /home/user/.local/share/zinc95
	root_path.data = getenv("HOME");
	if (root_path.data == NULL) {
		// Failed
		printf("Could not find home directory\n");
		return root_path;
	}
	root_path.len = strlen(root_path.data);

	root_path = path_append(allocator, root_path, STR("/.local/share/zinc95"));

	return root_path;
}

static string_t get_absolute_path(allocator_t allocator, string_t path) {
	return path_append(allocator, _get_root_path(get_temp_allocator()), path);
}

void create_directory(string_t path) {
	string_t_array_t array = string_split(get_temp_allocator(), path, '/');

	string_builder_t builder = {0};
	string_builder_init(&builder, get_temp_allocator(), PATH_MAX);

	string_builder_append(&builder, _get_root_path(get_temp_allocator()));
	string_builder_append(&builder, STR("/"));

	// We have to create every subdirectory from the root of path seperately
	for (size_t i = 0; i < array.len; i++) {
		// TODO: create some path functions
		string_builder_append(&builder, array.data[i]);
		string_builder_append(&builder, STR("/"));

		if (mkdir(string_to_c_string(get_temp_allocator(), builder.string), 0755) != 0) {
			// Failed
		}
	}
}

typedef enum file_or_directory {
	DIRECTORIES,
	FILES,
} file_or_directory_t;

static string_t_array_t _get_files_or_directories_in_path(allocator_t allocator, string_t path, file_or_directory_t file_or_directory) {
	string_t_array_t array = {0};
	array_init(&array, allocator);

	string_t absolute_path = get_absolute_path(get_temp_allocator(), path);

	DIR *dir = opendir(string_to_c_string(get_temp_allocator(), absolute_path));
	if (dir == NULL) {
		// Failed
		// TODO: handle
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		string_t full_path = path_append(get_temp_allocator(), absolute_path, STR(entry->d_name));

		struct stat st;

		if (file_or_directory == DIRECTORIES) {
			if (stat(string_to_c_string(get_temp_allocator(), full_path), &st) == 0 && S_ISDIR(st.st_mode)) {
				if (!string_eq(STR(entry->d_name), STR(".")) && !string_eq(STR(entry->d_name), STR(".."))) {
					string_t copy = string_copy(get_temp_allocator(), STR(entry->d_name));
					array_append(&array, copy);
				}
			}
		} else {
			if (stat(string_to_c_string(get_temp_allocator(), full_path), &st) == 0 && S_ISREG(st.st_mode)) {
				string_t copy = string_copy(get_temp_allocator(), STR(entry->d_name));
				array_append(&array, copy);
			}
		}
	}

	closedir(dir);

	return array;
}

string_t_array_t get_directories_in_path(allocator_t allocator, string_t path) {
	return _get_files_or_directories_in_path(allocator, path, DIRECTORIES);
}

string_t_array_t get_files_in_path(allocator_t allocator, string_t path) {
	return _get_files_or_directories_in_path(allocator, path, FILES);
}

void create_default_directories() {
	string_t root_path = {0};

	// Create /home/user/.local/share/zinc95
	root_path.data = getenv("HOME");
	if (root_path.data == NULL) {
		// Do something
		printf("Could not find home directory\n");
		return;
	}
	root_path.len = strlen(root_path.data);

	// TODO: error handling
	root_path = path_append(get_temp_allocator(), root_path, STR("/.local"));
	mkdir(string_to_c_string(get_temp_allocator(), root_path), 0755);
	
	root_path = path_append(get_temp_allocator(), root_path, STR("/share"));
	mkdir(string_to_c_string(get_temp_allocator(), root_path), 0755);
	
	root_path = path_append(get_temp_allocator(), root_path, STR("/zinc95"));
	mkdir(string_to_c_string(get_temp_allocator(), root_path), 0755);
	
	create_directory(STR("discs"));
	create_directory(STR("saves"));
}

bool path_is_dir(string_t path) {
	struct stat path_stat;

	if (stat(string_to_c_string(get_temp_allocator(), path), &path_stat) != 0) {
		// Directory doesn't exist
		return false;
	}

	return S_ISDIR(path_stat.st_mode);
}

bool path_is_file(string_t path) {
	struct stat path_stat;

	if (stat(string_to_c_string(get_temp_allocator(), path), &path_stat) != 0) {
		// File doesn't exist
		return false;
	}

	return S_ISREG(path_stat.st_mode);
}
