#include <hotc.h>

int get_file_last_modified(const char *filename)
{
	struct stat attr;
	if (stat(filename, &attr) != 0) {
		perror("stat");
		return -1; // Error occurred
	}
	return (int)attr.st_mtime; // Return last modified time as an integer
}

/**
 * @brief Create a directory and all necessary parent directories.
 *
 * @param dir The directory to create.
 * @return int 1 on success, -1 on failure.
 */
int mkdir_p(const char *dir)
{
	if (strnlen(dir, MAX_NAME_LENGTH) >= 256) {
		printf("|| ERROR || Directory name too long: %s\n", dir);
		return -1;
	}
	const char *p = dir;
	char tmp[256];
	char *slash = NULL;
	while (*p) {
		slash = strchr(p, '/');
		if (slash) {
			strncpy(tmp, dir, slash - dir);
			tmp[slash - dir] = '\0';
		} else {
			strcpy(tmp, dir);
		}
		if (MKDIR(tmp) != 0 && errno != EEXIST) {
			printf("|| ERROR || Failed to create directory: %s\n", tmp);
			return -1; // Failed to create directory
		}
		if (!slash) break;
		p = slash + 1;
	}
	return 1;
}

int file_exists(const char *filename)
{
	struct stat attr;
	return stat(filename, &attr) == 0;
}