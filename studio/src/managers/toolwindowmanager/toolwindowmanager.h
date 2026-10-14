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
#ifndef TOOLWINDOWMANAGER_H
#define TOOLWINDOWMANAGER_H

#include <components/splitter.h>
#include <components/widget.h>
#include <variant.h>

#include <vector>

class AbstractToolWindowManagerArea;
class ToolWindowManagerArea;
class Layout;

class ToolWindowManager : public Widget
{
    A_OBJECT(ToolWindowManager, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(bool, tabsClosable, ToolWindowManager::tabsClosable, ToolWindowManager::setTabsClosable)
    )
    A_METHODS(
        A_SIGNAL(ToolWindowManager::toolWindowVisibilityChanged),
        A_SIGNAL(ToolWindowManager::currentToolWindowChanged),
        A_SLOT(ToolWindowManager::onCurrentToolWindowChanged)
    )
    A_NOENUMS()

public:
    // TODO: Add native floating windows after Engine supports multiple windows.
    // TODO: Add arbitrary widgets on the left and right sides of tabs to UIKit TabBar.

    enum AreaType {
        LastUsedArea,
        NewFloatingArea,
        EmptySpaceArea,
        NoArea
    };

    enum ReferenceType {
        ReferenceAddTo,
        ReferenceLeftOf,
        ReferenceRightOf,
        ReferenceTopOf,
        ReferenceBottomOf
    };

    ToolWindowManager();
    ~ToolWindowManager() override;

    void activateToolWindow(Widget *toolWindow);

    void addToolWindow(Widget *toolWindow, AreaType area = LastUsedArea);
    void addToolWindow(Widget *toolWindow, ReferenceType reference,
                       AbstractToolWindowManagerArea *area);
    void addToolWindows(const std::vector<Widget *> &toolWindows, AreaType area = LastUsedArea);
    void addToolWindows(const std::vector<Widget *> &toolWindows, ReferenceType reference,
                        AbstractToolWindowManagerArea *area);

    void moveToolWindow(Widget *toolWindow, AreaType area = LastUsedArea);
    void moveToolWindow(Widget *toolWindow, ReferenceType reference,
                        AbstractToolWindowManagerArea *area);
    void moveToolWindows(const std::vector<Widget *> &toolWindows, AreaType area = LastUsedArea);
    void moveToolWindows(const std::vector<Widget *> &toolWindows, ReferenceType reference,
                         AbstractToolWindowManagerArea *area);

    AbstractToolWindowManagerArea *areaFor(Widget *toolWindow) const;
    void removeToolWindow(Widget *toolWindow);
    std::vector<Widget *> toolWindows() const;
    void hideToolWindow(Widget *toolWindow);

    Variant saveState() const;
    bool restoreState(const Variant &data);

    bool tabsClosable() const;
    void setTabsClosable(bool enabled);

public: // signals
    void toolWindowVisibilityChanged(Widget *toolWindow);
    void currentToolWindowChanged(Widget *toolWindow);

protected:
    void composeComponent() override;
    void update(const Vector2 &position) override;
    virtual Splitter *createSplitter();
    virtual AbstractToolWindowManagerArea *createArea();

private:
    AbstractToolWindowManagerArea *createAndSetupArea();
    Splitter *createAndSetupSplitter();
    Widget *rootWidget() const;
    bool splitArea(AbstractToolWindowManagerArea *target, AbstractToolWindowManagerArea *added,
                   ReferenceType reference);
    void updateDropTarget(const Vector2 &position);
    void finishToolWindowDrag(const Vector2 &position);
    void equalizeSplitter(Splitter *splitter);
    void parkToolWindows(const std::vector<Widget *> &toolWindows);
    void simplifyLayout();
    VariantMap saveWidgetState(Widget *widget) const;
    Widget *restoreWidgetState(const VariantMap &data);
    AbstractToolWindowManagerArea *restoreAreaState(const VariantMap &data);
    void onCurrentToolWindowChanged(Widget *toolWindow);

    std::vector<Widget *> m_toolWindows;
    std::vector<AbstractToolWindowManagerArea *> m_areas;
    std::vector<Splitter *> m_splitters;
    AbstractToolWindowManagerArea *m_lastUsedArea;
    AbstractToolWindowManagerArea *m_dropArea;
    Widget *m_draggedToolWindow;
    Vector2 m_dragStartPosition;
    ReferenceType m_dropReference;
    Layout *m_rootLayout;
    uint32_t m_areaCounter;
    uint32_t m_splitterCounter;
    bool m_tabsClosable;
    bool m_dragging;
};

#endif // TOOLWINDOWMANAGER_H
