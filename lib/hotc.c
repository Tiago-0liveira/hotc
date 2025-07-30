#include <hotc.h>

#ifdef _WIN32
    #include <windows.h>
	#include <conio.h>
#else
    #include <dlfcn.h>
	#include <unistd.h>
#endif

t_lib_info open_libs[MAX_OPEN_LIBS] = { 0 };
static const char *HOTC_PATH = "./";

void check_and_update_libs(update_lib_event_handlers event_handlers)
{
	int mod_count = 0;
	timestamp_t start = get_time_ms();
	for (int i = 0; i < MAX_OPEN_LIBS; i++) {
		t_lib_info *lib_info = &open_libs[i];
		if (lib_info->source_path[0] != '\0') {
			int last_modified = get_file_last_modified(lib_info->source_path);
			if (last_modified != lib_info->last_modified) {
				if (event_handlers.before_check) { event_handlers.before_check(lib_info); }
				if (lib_info->handle) {
					printf("|| INFO || Library %s has been modified, reloading...\n", lib_info->name);
				}
				if (event_handlers.before_rebuild) { event_handlers.before_rebuild(lib_info); }
				unload_shared_library(lib_info);
				
				remove(lib_info->lib_path); // Remove the old library file
				if (compile_shared_library(lib_info) == -1) {
					lib_info->last_modified = last_modified;
					if (event_handlers.on_error) { event_handlers.on_error(lib_info); }
					continue; // Ignore iteartion and try again after
				}
				
				load_shared_library(lib_info);
				if (event_handlers.after_rebuild) { event_handlers.after_rebuild(lib_info); }
				lib_info->last_modified = last_modified;
				mod_count++;
			}
			lib_info->last_modified = last_modified; // Update the last modified time
		}
	}
	if (mod_count > 0) {
		timestamp_t end = get_time_ms();
		printf("|| INFO || %d libraries were reloaded in %lld ms.\n", mod_count, end - start);
	}
}

/**
 * @brief Find an open library slot by its source path
 * This function searches for the next available slot in the open_libs array
 * and initializes the t_lib_info structure for that slot.
 * 
 * @param source_path The source file path
 * @param handlers Parameter of type event_handlers (can handle on_load and pre_unload)
 * @param extra_compiler_flags If you need any more flags, add here
 * @return t_lib_info* 
 */
t_lib_info *register_shared_library(const char *source_path, event_handlers handlers, const char *extra_compiler_flags)
{
	t_lib_info *lib_info = find_open_lib(source_path);
	if (lib_info) {
		printf("|| INFO || Library already loaded: %s\n", source_path);
		return NULL; // Library already loaded
	}

	for (int i = 0; i < MAX_OPEN_LIBS; i++) {
		t_lib_info *lib_info = &open_libs[i];
		if (lib_info->source_path[0] == '\0') {
			setup_library(lib_info, source_path);
			lib_info->handlers = handlers;
			if (extra_compiler_flags) {
				if (strnlen(extra_compiler_flags, MAX_COMPILE_FLAGS_LENGTH) >= MAX_COMPILE_FLAGS_LENGTH) {
					fprintf(stderr, "|| ERROR || extra_compiler_flags can only have %d!\n", MAX_COMPILE_FLAGS_LENGTH);
					exit(EXIT_FAILURE);
				}
				strncpy(lib_info->extra_compile_flags, extra_compiler_flags, MAX_COMPILE_FLAGS_LENGTH);
			}
			compile_shared_library(lib_info);

			return lib_info;
		}
	}

	return NULL; // No available slot found
}

void unregister_shared_library(const char *source_path)
{
	t_lib_info *lib_info = find_open_lib(source_path);
	if (lib_info) {
		// Not found is ignored
		return; // Library already loaded
	}
	cleanup_shared_library(lib_info);
}

void	load_shared_library(t_lib_info *lib_info)
{
	if (!lib_info || lib_info->handle) {
		return ;
	}
	#ifdef _WIN32
		lib_info->handle = LoadLibraryA(lib_info->lib_path);
		if (!lib_info->handle) {
			printf("LoadLibrary failed (%lu)\n", GetLastError());
			return ;
		}
	#else
		lib_info->handle = dlopen(lib_info->lib_path, RTLD_LAZY);
		if (!lib_info->handle) {
			printf("dlopen failed: %s\n", dlerror());
			return ;
		}
	#endif
	lib_info->last_modified = get_file_last_modified(lib_info->source_path);
	// printf("Library loaded successfully |%s| -> %s\n", lib_info->name, lib_info->source_path);
	if (lib_info->handlers.on_load) {
		lib_info->handlers.on_load(lib_info);
	}
}

int unload_shared_library(t_lib_info *lib_info)
{
	if (!lib_info || lib_info->handle == NULL) {
		// Ignore
		return 0;
	}
	func_ptr_event_handler pre_unload_func = lib_info->handlers.pre_unload;
	if (pre_unload_func) {
		pre_unload_func(lib_info);
	}

	#ifdef _WIN32
		if (!FreeLibrary((HMODULE)lib_info->handle)) {
			printf("FreeLibrary failed (%lu)\n", GetLastError());
			return -1;
		}
	#else
		if (dlclose(lib_info->handle) != 0) {
			printf("dlclose failed: %s\n", dlerror());
			return -1;
		}
	#endif

	reset_lib_info(lib_info);
	printf("Library unloaded successfully: %s\n", lib_info->name);
	return 1;
}

void *get_lib_address(const t_lib_info *lib_info, const char *symbol_name)
{
	if (!lib_info || !symbol_name) return NULL;

	void *symbol_address = NULL;
	#ifdef _WIN32
		symbol_address = (void *)GetProcAddress((HMODULE)lib_info->handle, symbol_name);
	#else
		symbol_address = dlsym(lib_info->handle, symbol_name);
	#endif

	if (!symbol_address)
	{
		#ifdef HOTC_FLAG_ERROR_ON_INVALID_SYMBOL
			printf("|| ERROR || Invalid symbol address for %s in library %s\n", symbol_name, lib_info->lib_path);
			exit(EXIT_FAILURE);
		#else
			return NULL;
		#endif
	}
	return symbol_address;
}

int compile_shared_library(const t_lib_info *lib_info)
{
	char command[2000];
	
	int count = 0;
	#ifdef _WIN32
		count = snprintf(command, sizeof(command), "gcc -shared -o %s %s -I %s%s", lib_info->lib_path, lib_info->source_path, HOTC_PATH, HOTC_INCLUDE_FINAL_PATH);
	#else
		count = snprintf(command, sizeof(command), "gcc -shared -fPIC -o %s %s -I %s%s", lib_info->lib_path, lib_info->source_path, HOTC_PATH, HOTC_INCLUDE_FINAL_PATH);
	#endif
	if (lib_info->extra_compile_flags[0] != '\0') {
		snprintf(command + count, sizeof(command) + count, " %s", lib_info->extra_compile_flags);
	}

	remove(lib_info->lib_path); // Remove old library file if it exists
	int result = system(command);
	if (result != 0) {
		printf("Failed to compile shared library: %s\n", lib_info->source_path);
		return -1; // Compilation failed
	}
	return 0; // Compilation successful
}

void HOTC_SET_PATH(const char *hotc_path)
{
	HOTC_PATH = hotc_path;
}
