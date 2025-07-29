/*
Dirty windows implementation of IO functions
*/

#include "io.h"

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <string.h>

#include "string.h"

// TODO: mostly replace string_concat with path_append (when I'm back on Windows)

static void _convert_to_windows_path(string_t path) {
	for (size_t i = 0; i < path.len; i++) {
		if (path.data[i] == '/') {
			path.data[i] = '\\';
		}
	}
}

static string_t _get_root_path(allocator_t allocator) {
	string_t path = {
		.data = alloc(allocator, MAX_PATH),
		.len = 0,
	};

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path.data))) {
		path.len = strlen(path.data);
		
		return string_concat(allocator, path, STR("\\zinc95"));
	}

	return path;
}

static string_t _get_absolute_path(allocator_t allocator, string_t path) {
	_convert_to_windows_path(path);

	// TODO: always use the temp allocator for root_path I think
	string_t root_path = _get_root_path(allocator);
	string_t temp = string_concat(allocator, root_path, STR("\\"));
	string_t absolute_path = string_concat(allocator, temp, path);

	return absolute_path;
}

static void _create_single_directory(string_t path) {
	string_t absolute_path = _get_absolute_path(get_temp_allocator(), path);

	printf("Trying to create directory ");
	print_string(absolute_path);
	printf("...\n");
	
	if (CreateDirectory(string_to_c_string(get_temp_allocator(), absolute_path), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Directory created or already exists: %s", absolute_path);
		print_string(absolute_path);
		printf("\n");
	} else {
		fprintf(stderr, "Failed to create directory. Error: %lu\n", GetLastError());
	}
}

void create_directory(string_t path) {
	string_t_array_t array = string_split(get_temp_allocator(), path, '/');

	string_builder_t builder = {0};
	string_builder_init(&builder, get_temp_allocator(), MAX_PATH);

	// We have to create every subdirectory from the root of path seperately
	for (size_t i = 0; i < array.len; i++) {
		// TODO: create some path functions
		string_builder_append(&builder, array.data[i]);
		string_builder_append(&builder, STR("\\"));

		_create_single_directory(builder.string);
	}
}

// Currently only prints
string_t_array_t get_directories_in_path(allocator_t allocator, string_t path) {
	string_t_array_t array = {0};
	array_init(&array, allocator);

	string_t absolute_path = _get_absolute_path(get_temp_allocator(), path);

	WIN32_FIND_DATA find_file_data;
	string_t search_path = temp_alloc_string(MAX_PATH);

	snprintf(search_path.data, MAX_PATH, "%s\\*", string_to_c_string(get_temp_allocator(), absolute_path));
	HANDLE h_find = FindFirstFile(search_path.data, &find_file_data);
	
	if (h_find == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("FindFirstFile failed. Error code: %lu\n", error);
		return array;
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

	string_t absolute_path = _get_absolute_path(get_temp_allocator(), path);

	WIN32_FIND_DATA find_file_data;
	string_t search_path = temp_alloc_string(MAX_PATH);

	snprintf(search_path.data, MAX_PATH, "%s\\*", string_to_c_string(get_temp_allocator(), absolute_path));
	HANDLE h_find = FindFirstFile(search_path.data, &find_file_data);

	if (h_find == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		printf("FindFirstFile failed. Error code: %lu\n", error);
		return array;
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
	// char path[MAX_PATH];
	string_t path = temp_alloc_string(MAX_PATH);

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path.data))) {
		path.len = strlen(path.data);

		printf("AppData path: %s\n", path.data);
		path = string_concat(get_temp_allocator(), path, STR("\\zinc95"));

		if (CreateDirectory(path.data, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("Root directory created or already exists: %s\n", path);
		} else {
			fprintf(stderr, "Failed to create directory. Error: %lu\n", GetLastError());
		}
	} else {
		fprintf(stderr, "Failed to get root path\n");
	}

	create_directory(STR("discs"));
	create_directory(STR("saves"));
}