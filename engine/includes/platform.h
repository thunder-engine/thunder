#ifdef _WIN32
    #include <Windows.h>

    #define THUNDER_MAIN() \
        int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) { \
            int result = 0; \
            int w_argc = 0; \
            LPWSTR *w_argv = CommandLineToArgvW(GetCommandLineW(), &w_argc); \
            if (w_argv) { \
                char **argv = new char*[w_argc]; \
                int argc = 0; \
                for (int i = 0; i < w_argc; ++i) { \
                    int w_len = lstrlenW(w_argv[i]); \
                    int len = WideCharToMultiByte(CP_ACP, 0, w_argv[i], w_len, nullptr, 0, nullptr, nullptr); \
                    argv[argc] = new char[len+1]; \
                    WideCharToMultiByte(CP_ACP, 0, w_argv[i], w_len, argv[argc], len+1, nullptr, nullptr); \
                    ++argc; \
                } \
                Engine *engine = new Engine(argv[0]); \
                result = thunderMain(engine); \
                delete engine; \
                for (int i = 0; i < argc; ++i) { \
                    delete []argv[i]; \
                } \
                delete []argv; \
                LocalFree(w_argv); \
            } \
            return result; \
        }
#elif defined (THUNDER_MOBILE)
    #define THUNDER_MAIN()
#else
    #define THUNDER_MAIN() \
    int main(int, char **argv) { \
        Engine *engine = new Engine(argv[0]); \
        thunderMain(engine); \
        delete engine; \
        return 0; \
    }
#endif
