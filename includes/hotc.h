#ifndef HOTC_H
#define HOTC_H

#include <hotc_io.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define HOTC_INCLUDE_FINAL_PATH "includes/public"
#ifndef HOTC_PATH
	#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
		#pragma message("You have to define HOTC_PATH otherwise you cant use <hotc_shared.h> inside your shared libraries!")
	#else
		#warning "You have to define HOTC_PATH"
	#endif
	#define HOTC_PATH "./"
#endif

typedef void* t_lib_handle;

// FLAGS
// This flag can be used to exit the program if an invalid symbol is encountered.(null address)
#define HOTC_FLAG_ERROR_ON_INVALID_SYMBOL

#define MAX_OPEN_LIBS 20
#define MAX_NAME_LENGTH 256
#define DLL_DIR "./__shared_libs/"
#define DLL_DIR_LEN 16

typedef unsigned long long timestamp_t;

struct s_lib_info;

typedef void (*func_ptr_event_handler)(const struct s_lib_info *lib_info);

typedef struct {
	func_ptr_event_handler on_load;
	func_ptr_event_handler pre_unload;
} event_handlers;

#define HOTC_DEFAULT_EVENTS (event_handlers) {NULL, NULL}
#define HOTC_ON_LOAD(handler) (event_handlers) {handler, NULL}
#define HOTC_PRE_UNLOAD(handler) (event_handlers) {NULL, handler}


typedef struct s_lib_info {
    int last_modified;
    char name[MAX_NAME_LENGTH];
    char source_path[MAX_NAME_LENGTH];
    char lib_path[MAX_NAME_LENGTH];
	event_handlers handlers;
    t_lib_handle handle;
} t_lib_info;


extern t_lib_info open_libs[MAX_OPEN_LIBS];

// hot.c
void		check_and_update_libs();
t_lib_info	*register_shared_library(const char *source_path, event_handlers handlers);
void 		unregister_shared_library(const char *source_path);
void		load_shared_library(t_lib_info *lib_info);
int			unload_shared_library(t_lib_info *lib_info);
void		*get_lib_address(const t_lib_info *lib_info, const char *symbol_name);
int			compile_shared_library(const t_lib_info *lib_info);

// utils.c
void		reset_lib_info(t_lib_info *lib_info);
t_lib_info	*find_open_lib(const char *source_path);
char		*get_source_filename(const char *path);
int			setup_library(t_lib_info *lib_info, const char *source_path);
char		*get_lib_path(const char *source_path, const char *filename);
void		cleanup_shared_library(t_lib_info *lib_info);
timestamp_t	get_time_ms();

#endif // HOTC_H