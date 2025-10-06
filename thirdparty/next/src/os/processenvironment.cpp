#include "processenvironment.h"

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
    #include <string>
#endif

ProcessEnvironment& ProcessEnvironment::operator=(const ProcessEnvironment &other) {
    if(this != &other) {
        m_envVars = other.m_envVars;
    }
    return *this;
}

void ProcessEnvironment::insert(const TString &key, const TString &value) {
#ifdef _WIN32
    m_envVars[key.toUpper()] = value;
#else
    m_envVars[key] = value;
#endif
}

TString ProcessEnvironment::value(const TString &key) const {
    auto it = m_envVars.find(key);
    if(it != m_envVars.end()) {
        return it->second;
    }
    return TString();
}

const std::map<TString, TString> &ProcessEnvironment::envVars() const {
    return m_envVars;
}

ProcessEnvironment ProcessEnvironment::systemEnvironment() {
    ProcessEnvironment env;

#ifdef _WIN32
    LPWCH envStrings = GetEnvironmentStringsW();
    if (envStrings) {
        LPWCH current = envStrings;
        while(*current) {
            std::wstring envStr(current);
            size_t pos = envStr.find(L'=');
            if(pos != std::wstring::npos) {
                std::string name = std::string(envStr.begin(), envStr.begin() + pos);
                std::string value = std::string(envStr.begin() + pos + 1, envStr.end());
                env.insert(name, value);
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
            env.insert(name, value);
        }
    }
#endif

    return env;
}
