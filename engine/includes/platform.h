#ifdef _WIN32
    #include <windows.h>

    #define THUNDER_MAIN() \
        int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) { \
            int result  = 0; \
            int w_argc  = 0; \
            LPWSTR *w_argv  = CommandLineToArgvW(GetCommandLineW(), &w_argc); \
            if (w_argv) { \
                char **argv = new char*[w_argc]; \
                int argc    = 0; \
                for (int i = 0; i < w_argc; ++i) { \
                    int w_len   = lstrlenW(w_argv[i]); \
                    int len     = WideCharToMultiByte(CP_ACP, 0, w_argv[i], w_len, NULL, 0, NULL, NULL); \
                    argv[argc]  = new char[len+1]; \
                    WideCharToMultiByte(CP_ACP, 0, w_argv[i], w_len, argv[argc], len+1, NULL, NULL); \
                    ++argc; \
                } \
                result  = main(argc, argv); \
                for (int i = 0; i < argc; ++i) { \
                    delete []argv[i]; \
                } \
                delete []argv; \
                LocalFree(w_argv); \
            } \
            return result; \
        }
#else
    #define THUNDER_MAIN()
#endif
