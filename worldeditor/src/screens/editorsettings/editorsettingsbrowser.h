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
#ifndef EDITORSETTINGSBROWSER_H
#define EDITORSETTINGSBROWSER_H

#include <QWidget>

#include <engine.h>

namespace Ui {
    class EditorSettingsBrowser;
}

class QAbstractItemModel;

class EditorSettingsBrowser : public QWidget {
    Q_OBJECT
public:
    explicit EditorSettingsBrowser(QWidget *parent = 0);
    ~EditorSettingsBrowser();

    void init();

private slots:
    void onSettingsUpdated(const Object::ObjectList &objects, const TString &property, Variant value);

    void on_groups_clicked(const QModelIndex &index);

private:
    void changeEvent(QEvent *event) override;

    Ui::EditorSettingsBrowser *ui;

    QAbstractItemModel *m_groupModel;

};

#endif // EDITORSETTINGSBROWSER_H
