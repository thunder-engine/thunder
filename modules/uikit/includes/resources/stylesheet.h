#ifndef STYLESHEET_H
#define STYLESHEET_H

#include <resource.h>
#include <uikit.h>

class Widget;

class UIKIT_EXPORT StyleSheet : public Resource {
    A_REGISTER(StyleSheet, Resource, Resources)

public:
    StyleSheet();

    bool addRawData(const string &data);

    void resolve(Widget *widget);

    static void resolveInline(Widget *widget, const string &style);

    static void setStyleProperty(Widget *widget, const string &key, const string &value);

    static Vector4 toColor(const string &value);
    static float toLength(const string &value, bool &pixels);

private:
    void *m_parser;

};

#endif // STYLESHEET_H
