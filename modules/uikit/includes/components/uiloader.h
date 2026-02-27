#ifndef UILOADER_H
#define UILOADER_H

#include "widget.h"

#include "stylesheet.h"
#include "uidocument.h"

class UIKIT_EXPORT UiLoader : public Widget {
    A_OBJECT(UiLoader, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(UiDocument *, document, UiLoader::document, UiLoader::setDocument, "editor=Asset"),
        A_PROPERTYEX(StyleSheet *, styleSheet, UiLoader::styleSheet, UiLoader::setStyleSheet, "editor=Asset")
    )
    A_METHODS(
        A_METHOD(void, UiLoader::fromBuffer),
        A_SIGNAL(UiLoader::documentLoaded)
    )
    A_NOENUMS()

public:
    UiLoader();

    UiDocument *document() const;
    void setDocument(UiDocument *document);

    StyleSheet *styleSheet() const;
    void setStyleSheet(StyleSheet *style);

    TString documentStyle() const;

    void fromBuffer(const TString &buffer);

    void documentLoaded();

private:
    void resolveStyleSheet(Widget *widget);

    void cleanHierarchy(Widget *widget);

private:
    TString m_documentStyle;

    UiDocument *m_document;

    StyleSheet *m_styleSheet;

};

#endif // UILOADER_H
