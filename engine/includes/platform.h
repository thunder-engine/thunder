/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
                result = thunderMain(); \
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
        thunderMain(); \
        return 0; \
    }
#endif
