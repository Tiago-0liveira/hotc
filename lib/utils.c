#include <hotc.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

void reset_lib_info(t_lib_info *lib_info)
{
	if (lib_info == NULL) return;

	lib_info->last_modified = -1;
	lib_info->handle = NULL;
	lib_info->name[0] = '\0'; // Clear the name
}

t_lib_info *find_open_lib(const char *source_path)
{
	for (int i = 0; i < MAX_OPEN_LIBS; i++) {
		t_lib_info *lib_info = &open_libs[i];
		if (strcmp(lib_info->source_path, source_path) == 0) {
			return lib_info;
		}
	}

	return NULL;
}

char *get_source_filename(const char *path)
{
	if (path == NULL) return NULL;
	if (strlen(path) >= MAX_NAME_LENGTH)
	{
		printf("|| ERROR || Path too long: %s\n", path);
		printf("|| ERROR || Max is: %d\n", MAX_NAME_LENGTH);
		return NULL;
	}
	char *slash = strrchr(path, '/');
	if (slash) {
		slash++;
	} else {
		slash = (char *)path; // No '/' found, use the whole path
	}
	char *dot = strchr(slash, '.');
	if (dot) {
		if (!(strncmp(dot, ".c", 2) == 0 || strncmp(dot, ".cpp", 4) == 0)) {
			printf("|| ERROR || Invalid file extension: %s\n", dot);
			printf("|| ERROR || Only .c and .cpp files are supported. (For now!?)\n");
			exit(EXIT_FAILURE);
		}
	}
	char *cleaned_name = malloc(dot - slash + 1);
	if (cleaned_name == NULL) {
		printf("|| ERROR || Memory allocation failed for cleaned_name.\n");
		exit(EXIT_FAILURE);
	}
	strncpy(cleaned_name, slash, dot - slash);
	cleaned_name[dot - slash] = '\0'; // Null-terminate the string
	return cleaned_name;
}

int setup_library(t_lib_info *lib_info, const char *source_path)
{
	if (lib_info == NULL || source_path == NULL) {
		printf("|| ERROR || Invalid arguments to setup_library.\n");
		return -1;
	}

	char *filename = get_source_filename(source_path);
	if (!filename) {
		fprintf(stderr, "Failed to get source filename from path: %s\n", source_path);
		return -1;
	}
	strncpy(lib_info->name, filename, MAX_NAME_LENGTH - 1);
	lib_info->name[MAX_NAME_LENGTH - 1] = '\0';
	free(filename);

	char *lib_path = get_lib_path(source_path, lib_info->name);
	if (!lib_path) {
		fprintf(stderr, "Failed to get library path for source: %s\n", source_path);
		return -1;
	}
	strncpy(lib_info->lib_path, lib_path, MAX_NAME_LENGTH - 1);
	lib_info->lib_path[MAX_NAME_LENGTH - 1] = '\0';
	free(lib_path);

	lib_info->last_modified = -1;
	lib_info->handle = NULL;

	strncpy(lib_info->source_path, source_path, MAX_NAME_LENGTH - 1);
	lib_info->source_path[MAX_NAME_LENGTH - 1] = '\0';
	return 1; // Success
}

char *get_lib_path(const char *source_path, const char *filename)
{
	if (source_path == NULL || filename == NULL) return NULL;

	char *lib_path = malloc(MAX_NAME_LENGTH * 2);
	if (lib_path == NULL) {
		printf("|| ERROR || Memory allocation failed for lib_path.\n");
		exit(EXIT_FAILURE);
	}
	char *slash = strrchr(source_path, '/');
	int until_slash_len = slash ? (slash - source_path + 1) : 0;
	strncpy(lib_path, DLL_DIR, DLL_DIR_LEN + 1);
	int source_path_offset = 0;
	if (slash) {
		while (until_slash_len >= source_path_offset && (source_path[source_path_offset] == '.' || source_path[source_path_offset] == '/')) {
			source_path_offset++;
		}
	}
	strncpy(lib_path + DLL_DIR_LEN, source_path + source_path_offset, until_slash_len - source_path_offset);
	lib_path[DLL_DIR_LEN + until_slash_len - source_path_offset] = '\0';/* close string so we can mkdir */
	mkdir_p(lib_path);/* make all necessary parent dirs */

	#ifdef _WIN32
		snprintf(lib_path + DLL_DIR_LEN + until_slash_len - source_path_offset, MAX_NAME_LENGTH * 2 - DLL_DIR_LEN - until_slash_len + source_path_offset, "lib%s.dll", filename);
	#else
		snprintf(lib_path + DLL_DIR_LEN + until_slash_len - source_path_offset, MAX_NAME_LENGTH * 2 - DLL_DIR_LEN - until_slash_len + source_path_offset, "lib%s.so", filename);
	#endif

	return lib_path;
}

void cleanup_shared_library(t_lib_info *lib_info)
{
	if (!lib_info) return;

	unload_shared_library(lib_info);
	memset(lib_info->name, 0, sizeof(lib_info->name));
	memset(lib_info->lib_path, 0, sizeof(lib_info->lib_path));
	memset(lib_info->source_path, 0, sizeof(lib_info->source_path));
}

timestamp_t	get_time_ms()
{
	#ifdef _WIN32
		return GetTickCount64();
	#else
		struct timeval tv;
        gettimeofday(&tv, NULL);
        return (timestamp_t)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
	#endif	
}