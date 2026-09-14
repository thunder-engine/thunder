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
#ifndef COMPONENTBROWSER_H
#define COMPONENTBROWSER_H

#include <QWidget>

#include <astring.h>

class ComponentFilter;

namespace Ui {
    class ComponentBrowser;
}

class ComponentBrowser : public QWidget {
    Q_OBJECT
public:
    ComponentBrowser(QWidget *parent = 0);

    void setGroups(const StringList &groups = StringList());

signals:
    void componentSelected(const QString &uri);

private slots:
    void on_findComponent_textChanged(const QString &arg1);

    void on_componentsTree_clicked(const QModelIndex &index);

private:
    void changeEvent(QEvent *event) override;

    Ui::ComponentBrowser *ui;

    ComponentFilter *m_proxyModel;

};

#endif // COMPONENTBROWSER_H
