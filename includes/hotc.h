#ifndef HOTC_H
#define HOTC_H

#include <hotc_io.h>

#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
	#include <conio.h>
#else
    #include <dlfcn.h>
	#include <unistd.h>
#endif

#ifdef _WIN32
	typedef HMODULE t_lib_handle;
#else
	typedef void* t_lib_handle;
#endif

// FLAGS
// This flag can be used to exit the program if an invalid symbol is encountered.(null address)
#define HOTC_FLAG_ERROR_ON_INVALID_SYMBOL

#define MAX_OPEN_LIBS 20
#define MAX_NAME_LENGTH 256
#define DLL_DIR "./__shared_libs/"
#define DLL_DIR_LEN 16

typedef struct {
	int last_modified;
	char name[MAX_NAME_LENGTH];
	char source_path[MAX_NAME_LENGTH];
	char lib_path[MAX_NAME_LENGTH];
	t_lib_handle handle;
} t_lib_info;

extern t_lib_info open_libs[MAX_OPEN_LIBS];

// hot.c
void check_and_update_libs();
t_lib_info *load_shared_library(const char *path);
int unload_shared_library(const char *path);
void *get_lib_address(t_lib_info *lib_info, const char *symbol_name);
int compile_shared_library(t_lib_info *lib_info);

// utils.c
void reset_lib_info(t_lib_info *lib_info);
t_lib_info* find_open_lib(const char *name);
t_lib_info* find_next_lib(const char *name);
char *get_source_filename(const char *path);
int setup_library(t_lib_info *lib_info, const char *source_path);
char *get_lib_path(const char *source_path, const char *filename);


#endif // HOTC_H