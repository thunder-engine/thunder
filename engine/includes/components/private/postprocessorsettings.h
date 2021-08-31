#ifndef POSTPROCESSORSETTINGS_H
#define POSTPROCESSORSETTINGS_H

#include <string>

#include <variant.h>

using namespace std;

namespace {
    static map<string, Variant> s_settingsMap;
};

class PostProcessSettings {
public:
    PostProcessSettings();

    static map<string, Variant> &settings();

    static void registerSetting(const string &name, const Variant &value);

    Variant readValue(const string &name) const;
    void writeValue(const string &name, const Variant &value);

    void lerp(const PostProcessSettings &settings, float t);

    void resetDefault();

private:
    unordered_map<string, Variant> m_currentValues;

};

#endif // POSTPROCESSORSETTINGS_H
