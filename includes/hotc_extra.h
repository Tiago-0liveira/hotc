#ifndef HOTC_EXTRA_H
#define HOTC_EXTRA_H

#include <hotc.h>

#define LAMBDA(name, body) static void LAMBDA_##name(const t_lib_info *lib_info) body

#endif // HOTC_EXTRA_H