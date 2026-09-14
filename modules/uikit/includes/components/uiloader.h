/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
