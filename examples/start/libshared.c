#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

EXPORT const char* print_message() 
{
    return "Another k12mdk1m2dk1md1212d12d12d2dm";
}