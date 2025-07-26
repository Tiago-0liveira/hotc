#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

EXPORT const char* do_some() 
{
    return "ASKDmaksdmkas mdkamsd1212d12d dkma skmd kasmdkmasd";
}