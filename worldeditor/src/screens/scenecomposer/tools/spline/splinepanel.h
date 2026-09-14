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
#ifndef SPLINEPANEL_H
#define SPLINEPANEL_H

#include <QWidget>

class SplineTool;

namespace Ui {
    class SplinePanel;
}

class SplinePanel : public QWidget {
    Q_OBJECT

public:
    explicit SplinePanel(QWidget *parent = nullptr);
    ~SplinePanel();

    void setTool(SplineTool *tool);

    void update();

private slots:
    void onEditFinished();

    void on_breakPoint_clicked();

    void on_addPoint_clicked();

    void on_deletePoint_clicked();

private:
    Ui::SplinePanel *ui;

    SplineTool *m_tool;

};

#endif // SPLINEPANEL_H
