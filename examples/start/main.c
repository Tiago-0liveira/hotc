#include <hotc.h>
#include <stdio.h>
#include <hotc_extra.h>
#include <windows.h>

typedef const char* (*PrintMessageFunc)();
PrintMessageFunc print_message_ptr = NULL;
PrintMessageFunc do_some_ptr = NULL;

LAMBDA(libshared, {
	print_message_ptr = get_lib_address(lib_info, "print_message");
})
LAMBDA(lib2, {
	do_some_ptr = get_lib_address(lib_info, "do_some");
})


int main(int argc, char **argv)
{
	(void)argc;(void)argv;

	register_shared_library("./examples/start/libshared.c", HOTC_ON_LOAD(LAMBDA_libshared));
	register_shared_library("./examples/start/lib2.c", HOTC_ON_LOAD(LAMBDA_lib2));

	while (1) {
		check_and_update_libs();

		if (print_message_ptr)
			printf("libshared.c :: %s\n", print_message_ptr());
		if(do_some_ptr)
			printf("libshared.c :: %s\n", do_some_ptr());

		#ifdef _WIN32
			Sleep(1000);
		#else
			sleep(1);
		#endif
	}


	return 0;
}