#ifndef POSTPROCESSORSETTINGS_H
#define POSTPROCESSORSETTINGS_H

#include <variant.h>

class PostProcessSettings {
public:
    PostProcessSettings();

    static std::map<std::string, Variant> &settings();

    static void registerSetting(const std::string &name, const Variant &value);

    static Variant defaultValue(const std::string &name);

    Variant readValue(const std::string &name) const;
    void writeValue(const std::string &name, const Variant &value);

    void resetDefault();

private:
    std::unordered_map<std::string, Variant> m_currentValues;

    static std::map<std::string, Variant> s_settingsMap;
};

#endif // POSTPROCESSORSETTINGS_H
