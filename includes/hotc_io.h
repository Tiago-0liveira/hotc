#ifndef HOTC_IO_H
#define HOTC_IO_H

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
    #include <direct.h>   // _mkdir
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/stat.h> // mkdir
    #include <sys/types.h>
    #define MKDIR(dir) mkdir(dir, 0755)
#endif

int	get_file_last_modified(const char *filename);
int	mkdir_p(const char *dir);
int	file_exists(const char *filename);

#endif // HOTC_IO_H