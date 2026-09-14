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
