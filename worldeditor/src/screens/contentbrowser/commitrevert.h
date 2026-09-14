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
#ifndef COMMITREVERT_H
#define COMMITREVERT_H

#include <QWidget>

#include <engine.h>

class CommitRevertProxy;
class AssetConverterSettings;

namespace Ui {
    class CommitRevert;
}

class CommitRevert : public QWidget {
    Q_OBJECT

public:
    explicit CommitRevert(QWidget *parent = nullptr);
    ~CommitRevert();

    void setObject(Object *object);

    void onSettingsUpdated();

    void checkImportSettings(AssetConverterSettings *settings);

signals:
    void reverted();

private slots:
    void on_commitButton_clicked();
    void on_revertButton_clicked();

private:
    Ui::CommitRevert *ui;

    Object *m_propertyObject;

    CommitRevertProxy *m_proxy;

};

class CommitRevertProxy : public Object {
    A_OBJECT(CommitRevertProxy, Object, Proxy)

    A_METHODS(
        A_SLOT(CommitRevertProxy::onUpdated)
    )
public:
    void setEditor(CommitRevert *editor) {
        m_editor = editor;
    }

    void onUpdated() {
        m_editor->onSettingsUpdated();
    }

private:
    CommitRevert *m_editor = nullptr;

};

#endif // COMMITREVERT_H
