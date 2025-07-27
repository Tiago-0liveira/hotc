#include <hotc.h>

t_lib_info open_libs[MAX_OPEN_LIBS] = { 0 };

void check_and_update_libs()
{
	int mod_count = 0;
	for (int i = 0; i < MAX_OPEN_LIBS; i++) {
		t_lib_info *lib_info = &open_libs[i];
		if (lib_info->handle != NULL) {
			int last_modified = get_file_last_modified(lib_info->source_path);
			if (last_modified != lib_info->last_modified) {
				printf("|| INFO || Library %s has been modified, reloading...\n", lib_info->name);
				unload_shared_library(lib_info->source_path);
				
				remove(lib_info->lib_path); // Remove the old library file
				compile_shared_library(lib_info);
				
				load_shared_library(lib_info->source_path);
				lib_info->last_modified = last_modified;
				mod_count++;
			}
			lib_info->last_modified = last_modified; // Update the last modified time
		}
	}
	if (mod_count > 0) {
		printf("|| INFO || %d libraries were reloaded due to modifications.\n", mod_count);
	}
}

t_lib_info	*load_shared_library(const char *source_path)
{
	t_lib_info *lib_info = find_open_lib(source_path);
	if (lib_info) {
		printf("|| INFO || Library already loaded: %s\n", source_path);
		return NULL; // Library already loaded
	}
	lib_info = find_next_lib(source_path);
	if (!lib_info) {
		printf("|| ERROR || No available slot for library: %s\n", source_path);
		printf("|| ERROR || All slots full. Max is: %d\n", MAX_OPEN_LIBS);
		return NULL; // No available slot
	}
	#ifdef _WIN32
		lib_info->handle = LoadLibraryA(lib_info->lib_path);
		if (!lib_info->handle) {
			printf("LoadLibrary failed (%lu)\n", GetLastError());
			return NULL;
		}
	#else
		lib_info->handle = dlopen(lib_info->lib_path, RTLD_LAZY);
		if (!lib_info->handle) {
			printf("dlopen failed: %s\n", dlerror());
			return NULL;
		}
	#endif
	lib_info->last_modified = get_file_last_modified(lib_info->source_path);
	printf("Library loaded successfully |%s| -> %s\n", lib_info->name, lib_info->source_path);
	return lib_info;
}

int unload_shared_library(const char *source_path)
{
	t_lib_info *lib_info = find_open_lib(source_path);
	if (!lib_info) {
		return 0; // Library not loaded
	}

	#ifdef _WIN32
		if (!FreeLibrary(lib_info->handle)) {
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
	printf("Library unloaded successfully: %s\n", source_path);
	return 1;
}

void *get_lib_address(t_lib_info *lib_info, const char *symbol_name)
{
	if (!lib_info || !symbol_name) return NULL;

	void *symbol_address = NULL;
	#ifdef _WIN32
		symbol_address = (void *)GetProcAddress(lib_info->handle, symbol_name);
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

int compile_shared_library(t_lib_info *lib_info)
{
	char command[700];
	
	#ifdef _WIN32
		snprintf(command, sizeof(command), "gcc -shared -o %s %s", lib_info->lib_path, lib_info->source_path);
	#else
		snprintf(command, sizeof(command), "gcc -shared -fPIC -o %s %s", lib_info->lib_path, lib_info->source_path);
	#endif

	remove(lib_info->lib_path); // Remove old library file if it exists
	int result = system(command);
	if (result != 0) {
		printf("Failed to compile shared library: %s\n", lib_info->source_path);
		return -1; // Compilation failed
	}
	return 0; // Compilation successful
}
