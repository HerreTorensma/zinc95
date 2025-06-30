/*
Dirty windows implementation of IO functions
*/

#include "io.h"

#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <string.h>

static void _convert_to_windows_path(char path[]) {
	size_t len = strlen(path);
	for (size_t i = 0; i < len; i++) {
		if (path[i] == '/') {
			path[i] = '\\';
		}
	}
}

static int _get_root_path(char path[]) {
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) {
		strcat(path, "\\zinc95");
		return 1;
	}

	return 0;
}

static int _get_discs_path(char path[]) {
	_get_root_path(path);
	strcat(path, "\\discs");
}

void create_directory(char path[]) {
	_convert_to_windows_path(path);
	
	char absolute_path[MAX_PATH];
	if (_get_root_path(absolute_path)) {
		strcat(absolute_path, "\\");
		strcat(absolute_path, path);
	
		if (CreateDirectory(absolute_path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("Directory created or already exists: %s\n", absolute_path);
		} else {
			fprintf(stderr, "Failed to create directory. Error: %lu\n", GetLastError());
		}
	} else {
		fprintf(stderr, "Failed to get the root directory\n");
	}
}

void list(char path[]) {

}

// void create_directory_in_discs(char path[]) {
// 	create_directory(path);
// }

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

	create_directory("discs");
	create_directory("saves");

	return 0;
}