#ifndef STYLESHEET_H
#define STYLESHEET_H

#include <resource.h>
#include <uikit.h>

class Widget;

class UIKIT_EXPORT StyleSheet : public Resource {
    A_OBJECT(StyleSheet, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(bool, StyleSheet::addRawData)
    )
    A_NOENUMS()

public:
    StyleSheet();

    String data() const;
    void setData(const String &data);

    bool addRawData(const String &data);

    void resolve(Widget *widget);

    static void resolveInline(Widget *widget, const String &style);

    static void setStyleProperty(Widget *widget, const String &key, const String &value);

    static Vector4 toColor(const String &value);
    static float toLength(const String &value, bool &pixels);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    String m_data;

    void *m_parser;

};

#endif // STYLESHEET_H
