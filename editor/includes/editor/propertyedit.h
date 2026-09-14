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
#ifndef PROPERTYEDIT_H
#define PROPERTYEDIT_H

#include <QWidget>

#include <editor.h>

class EDITOR_EXPORT PropertyEdit : public QWidget {
    Q_OBJECT

public:
    typedef PropertyEdit*(*UserTypeCallback)(int userType, QWidget *parent, const TString &editor);

public:
    explicit PropertyEdit(QWidget *parent = nullptr);
    ~PropertyEdit();

    virtual Variant data() const;
    virtual void setData(const Variant &data);

    virtual void setMixedValue(bool mixed);
    bool isMixedValue() const;

    virtual void setEditorHint(const TString &hint);

    virtual void setObject(Object *object, const TString &property);

    static void registerEditorFactory(UserTypeCallback callback);

    static void unregisterEditorFactory(UserTypeCallback callback);

    static PropertyEdit *constructEditor(int userType, QWidget *parent, const TString &editor);

signals:
    void dataChanged();
    void editFinished();

protected:
    static std::list<UserTypeCallback> m_userCallbacks;

    Object *m_object;
    bool m_mixedValue;

};

#endif // PROPERTYEDIT_H
