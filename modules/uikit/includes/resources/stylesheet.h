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

    TString data() const;
    void setData(const TString &data);

    bool addRawData(const TString &data);

    void resolve(Widget *widget);

    static void resolveInline(Widget *widget, const TString &style);

    static void setStyleProperty(Widget *widget, const TString &key, const TString &value);

    static Vector4 toColor(const TString &value);
    static TString toColor(const Vector4 &value);

    static float toLength(const TString &value, bool &pixels);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    TString m_data;

    void *m_parser;

};

#endif // STYLESHEET_H
