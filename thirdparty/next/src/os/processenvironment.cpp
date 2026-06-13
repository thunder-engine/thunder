#include "processenvironment.h"

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
    #include <string>
#endif

/*!
    \class ProcessEnvironment
    \brief Helper class for managing environment variables passed to child processes.
    \since Next 1.0
    \inmodule OS
*/

/*!
    Copy-assign environment variables from another instance.

    Performs a deep copy of the internal environment variable map.
*/
ProcessEnvironment &ProcessEnvironment::operator=(const ProcessEnvironment &other) {
    if(this != &other) {
        m_envVars = other.m_envVars;
    }
    return *this;
}

/*!
    Insert or update an environment \a variable.

    On Windows the \a key is normalized to upper-case prior to insertion.
*/
void ProcessEnvironment::insert(const TString &key, const TString &variable) {
#ifdef _WIN32
    m_envVars[key.toUpper()] = variable;
#else
    m_envVars[key] = variable;
#endif
}

/*!
    Return the value for \a key or an empty string if not found.
*/
TString ProcessEnvironment::value(const TString &key) const {
    auto it = m_envVars.find(key);
    if(it != m_envVars.end()) {
        return it->second;
    }
    return TString();
}

/*!
    Access the underlying map of environment variables.
*/
const std::map<TString, TString> &ProcessEnvironment::envVars() const {
    return m_envVars;
}

/*!
    Create a ProcessEnvironment populated from the current system environment.

    On Windows reads the wide-character environment block returned by
    `GetEnvironmentStringsW()`. On POSIX platforms it reads entries from
    the global `environ` pointer.
*/
ProcessEnvironment ProcessEnvironment::systemEnvironment() {
    ProcessEnvironment result;

#ifdef _WIN32
    LPWCH envStrings = GetEnvironmentStringsW();
    if(envStrings) {
        LPWCH current = envStrings;
        while(*current) {
            std::wstring envStr(current);
            size_t pos = envStr.find(L'=');
            if(pos != std::wstring::npos) {
                std::string name = std::string(envStr.begin(), envStr.begin() + pos);
                std::string value = std::string(envStr.begin() + pos + 1, envStr.end());
                result.insert(name, value);
            }
            current += envStr.length() + 1;
        }
        FreeEnvironmentStringsW(envStrings);
    }
#else
    extern char **environ;
    for(char **env = environ; *env != nullptr; env++) {
        std::string envStr(*env);
        size_t pos = envStr.find('=');
        if(pos != std::string::npos) {
            std::string name = envStr.substr(0, pos);
            std::string value = envStr.substr(pos + 1);
            result.insert(name, value);
        }
    }
#endif

    return result;
}
