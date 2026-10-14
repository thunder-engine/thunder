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
#ifndef ABSTRACTTOOLWINDOWMANAGERAREA_H
#define ABSTRACTTOOLWINDOWMANAGERAREA_H

#include <components/widget.h>
#include <variant.h>

#include <vector>

class ToolWindowManager;

class AbstractToolWindowManagerArea : public Widget
{
    A_OBJECT(AbstractToolWindowManagerArea, Widget, Components/UI)

    A_METHODS(
        A_SIGNAL(AbstractToolWindowManagerArea::currentToolWindowChanged),
        A_SLOT(AbstractToolWindowManagerArea::onCurrentIndexChanged),
        A_SLOT(AbstractToolWindowManagerArea::onTabCloseRequested)
    )
    A_NOENUMS()

public:
    AbstractToolWindowManagerArea();

    ToolWindowManager *manager() const;
    std::vector<Widget *> toolWindows() const;
    Widget *toolWindowAt(const Vector2 &position) const;
    void activateToolWindow(Widget *toolWindow);
    void addToolWindows(const std::vector<Widget *> &toolWindows);
    void removeToolWindow(Widget *toolWindow);
    void setTabsClosable(bool enabled);

    virtual Variant saveState() const;
    virtual void restoreState(const Variant &state);

    void currentToolWindowChanged(Widget *toolWindow);

protected:
    void composeComponent() override;
    void setManager(ToolWindowManager *manager);
    void onCurrentIndexChanged(int index);
    void onTabCloseRequested(int index);

    ToolWindowManager *m_manager;
    class TabWidget *m_tabWidget;
    class TabBar *m_tabBar;

    friend class ToolWindowManager;
};

#endif // ABSTRACTTOOLWINDOWMANAGERAREA_H
