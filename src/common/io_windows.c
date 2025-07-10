/*
Dirty windows implementation of IO functions
*/

#include "io.h"

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <string.h>

static void _convert_to_windows_path(string_t path) {
	for (size_t i = 0; i < path.len; i++) {
		if (path.data[i] == '/') {
			path.data[i] = '\\';
		}
	}
}

static int _get_temp_root_path(string_t *path) {
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path->data))) {
		path->len = strlen(path->data);
		string_append(path, STR("\\zinc95"));
		// return path;
	}

	return 0;
}

static string_t _get_absolute_path(string_t path) {
	_convert_to_windows_path(path);

	string_t absolute_path = temp_alloc_string(MAX_PATH);
	_get_temp_root_path(&absolute_path);
	string_append(&absolute_path, STR("\\"));
	string_append(&absolute_path, path);

	return absolute_path;
}

static void _create_single_directory(string_t path) {
	string_t absolute_path = _get_absolute_path(path);

	printf("Trying to create directory ");
	print_string(absolute_path);
	printf("...\n");
	
	if (CreateDirectory(string_to_c_string(get_temp_allocator(), absolute_path), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Directory created or already exists: ", absolute_path);
		print_string(absolute_path);
		printf("\n");
	} else {
		fprintf(stderr, "Failed to create directory. Error: %lu\n", GetLastError());
	}
}

void create_directory(string_t path) {
	// TODO: use string builder
	string_t_array_t array = string_split(get_temp_allocator(), path, '/');

	string_t base_path = temp_alloc_string(MAX_PATH);

	for (size_t i = 0; i < array.len; i++) {
		// TODO: create some path functions
		string_append(&base_path, array.data[i]);
		string_append(&base_path, STR("\\"));

		_create_single_directory(base_path);
	}
}

// Currently only prints
string_t_array_t get_directories_in_path(allocator_t allocator, string_t path) {
	string_t_array_t array = {0};
	array_init(&array, allocator);

	string_t absolute_path = _get_absolute_path(path);

	WIN32_FIND_DATA find_file_data;
	string_t search_path = temp_alloc_string(MAX_PATH);

	snprintf(search_path.data, MAX_PATH, "%s\\*", string_to_c_string(get_temp_allocator(), absolute_path));
	HANDLE h_find = FindFirstFile(search_path.data, &find_file_data);
	
	if (h_find == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("FindFirstFile failed. Error code: %lu\n", error);
		return;
	}

	do {
		if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			string_t copy = string_copy(get_temp_allocator(), STR(find_file_data.cFileName));
			array_append(&array, copy);
		}
	} while (FindNextFile(h_find, &find_file_data));

	FindClose(h_find);

	return array;
}

// Currently only prints
string_t_array_t get_files_in_path(allocator_t allocator, string_t path) {
	string_t_array_t array = {0};
	array_init(&array, allocator);

	string_t absolute_path = _get_absolute_path(path);

	WIN32_FIND_DATA find_file_data;
	string_t search_path = temp_alloc_string(MAX_PATH);

	snprintf(search_path.data, MAX_PATH, "%s\\*", string_to_c_string(get_temp_allocator(), absolute_path));
	HANDLE h_find = FindFirstFile(search_path.data, &find_file_data);

	if (h_find == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("FindFirstFile failed. Error code: %lu\n", error);
		return;
	}

	do {
		if (!(find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			string_t copy = string_copy(get_temp_allocator(), STR(find_file_data.cFileName));
			array_append(&array, copy);
		}
	} while (FindNextFile(h_find, &find_file_data));

	FindClose(h_find);

	return array;
}


void create_default_directories() {
	char path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
		printf("AppData path: %s\n", path);
		strcat(path, "\\zinc95");

		if (CreateDirectory(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("Root directory created or already exists: %s\n", path);
		} else {
			fprintf(stderr, "Failed to create directory. Error: %lu\n", GetLastError());
		}
	} else {
		fprintf(stderr, "Failed to get root path\n");
	}

	create_directory(STR("discs"));
	create_directory(STR("saves"));
	// create_directory(STR("a/test"));
	// _create_single_directory(STR("a/test"));

	// string_get_directories_in_path(STR("discs"));
	// get_files_in_path(STR("discs"));
}