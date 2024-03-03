#ifndef UILOADER_H
#define UILOADER_H

#include "widget.h"

#include "uidocument.h"
#include "stylesheet.h"

class UIKIT_EXPORT UiLoader : public Widget {
    A_REGISTER(UiLoader, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(UiDocument *, document, UiLoader::styleSheet, UiLoader::setStyleSheet, "editor=Asset"),
        A_PROPERTYEX(StyleSheet *, styleSheet, UiLoader::styleSheet, UiLoader::setStyleSheet, "editor=Asset")
    )
    A_METHODS(
        A_METHOD(void, UiLoader::fromBuffer)
    )

public:
    UiLoader();

    UiDocument *document() const;
    void setUiDocument(UiDocument *document);

    StyleSheet *styleSheet() const;
    void setStyleSheet(StyleSheet *style);

    string documentStyle() const;

    void fromBuffer(const string &buffer);

private:
    void resolveStyleSheet(Widget *widget);

    void cleanHierarchy(Widget *widget);

private:
    string m_documentStyle;

    UiDocument *m_document;

    StyleSheet *m_styleSheet;

};

#endif // UILOADER_H
