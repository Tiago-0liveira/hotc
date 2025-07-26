#include <hotc.h>
#include <stdio.h>


typedef const char* (*PrintMessageFunc)();
PrintMessageFunc print_message_ptr = NULL;
PrintMessageFunc do_some_ptr = NULL;

int main(int argc, char **argv)
{
	(void)argc;(void)argv;

	t_lib_info *lib_info = load_shared_library("./examples/start/libshared.c");
	if (!lib_info) {
		printf("Failed to load shared library.\n");
		return -1;
	}

	t_lib_info *lib_info2 = load_shared_library("./examples/start/lib2.c");
	if (!lib_info2) {
		printf("Failed to load shared library.\n");
		return -1;
	}

	while (1) {
		check_and_update_libs();

		print_message_ptr = (PrintMessageFunc)get_lib_address(lib_info, "print_message");
		if (print_message_ptr) {
			const char *message = print_message_ptr();
			printf("Message from shared library: %s\n", message);
		}

		do_some_ptr = (PrintMessageFunc)get_lib_address(lib_info2, "do_some");
		if (do_some_ptr) {
			const char *message = do_some_ptr();
			printf("Message from shared library2: %s\n", message);
		}

		#ifdef _WIN32
			Sleep(1000);
		#else
			sleep(1);
		#endif
	}


	return 0;
}