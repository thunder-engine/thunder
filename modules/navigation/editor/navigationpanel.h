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
#ifndef NAVIGATIONPANEL_H
#define NAVIGATIONPANEL_H

#include <editor/editorgadget.h>

#include <navigationsystem.h>

namespace Ui {
    class NavigationPanel;
}

class NavigationPanel : public EditorGadget {
    Q_OBJECT

public:
    explicit NavigationPanel(QWidget *parent = nullptr);
    ~NavigationPanel();

private:
    void loadSettings();
    void saveSettings();
    void updateEditor();
    void updateAgentType();

private slots:
    void onTypeSelected(int row);
    void onAddType();
    void onRemoveType();

private:
    void onUpdated() override {}

    void onSelectionChanged() override {}
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override {}

private:
    Ui::NavigationPanel *ui;
    std::vector<AgentType> m_agentTypes;
    bool m_updating = false;

};

#endif // NAVIGATIONPANEL_H
