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
#ifndef PROJECTBROWSER_H
#define PROJECTBROWSER_H

#include <QDialog>

#include <astring.h>

namespace Ui {
    class ProjectBrowser;
}

class ProjectBrowser : public QDialog {
    Q_OBJECT

public:
    explicit ProjectBrowser(QWidget *parent = nullptr);
    ~ProjectBrowser();

    TString projectPath() const { return m_projectPath; }

private:
    void onNewProject();
    void onImportProject();
    void onOpenProject();

    void onProjectSelected(const QModelIndex &index);

    void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::ProjectBrowser *ui;

    TString m_projectPath;
};

#endif // PROJECTBROWSER_H
