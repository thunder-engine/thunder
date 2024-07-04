#ifndef STYLESHEET_H
#define STYLESHEET_H

#include <resource.h>
#include <uikit.h>

class Widget;

class UIKIT_EXPORT StyleSheet : public Resource {
    A_REGISTER(StyleSheet, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(bool, StyleSheet::addRawData)
    )
    A_NOENUMS()

public:
    StyleSheet();

    std::string data() const;
    void setData(const std::string &data);

    bool addRawData(const std::string &data);

    void resolve(Widget *widget);

    static void resolveInline(Widget *widget, const std::string &style);

    static void setStyleProperty(Widget *widget, const std::string &key, const std::string &value);

    static Vector4 toColor(const std::string &value);
    static float toLength(const std::string &value, bool &pixels);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    std::string m_data;

    void *m_parser;

};

#endif // STYLESHEET_H
