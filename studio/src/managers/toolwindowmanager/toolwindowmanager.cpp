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
#include "toolwindowmanager.h"

#include "abstracttoolwindowmanagerarea.h"
#include "private/toolwindowmanagerarea_p.h"

#include <actor.h>
#include <layout.h>
#include <engine.h>
#include <input.h>
#include <log.h>

#include <algorithm>
#include <set>

namespace {
    const TString kStateFormat("QToolWindowManagerStateFormat");
    const TString kRoot("root");
    const TString kType("type");
    const TString kArea("area");
    const TString kSplitter("splitter");
    const TString kItems("items");
    const TString kSizes("sizes");
    const TString kOrientation("orientation");
    const TString kObjectNames("objectNames");
    const TString kCustomData("customData");
}

ToolWindowManager::ToolWindowManager() :
        m_lastUsedArea(nullptr),
        m_dropArea(nullptr),
        m_draggedToolWindow(nullptr),
        m_dragStartPosition(0.0f),
        m_dropReference(ReferenceAddTo),
        m_rootLayout(nullptr),
        m_areaCounter(0),
        m_splitterCounter(0),
        m_tabsClosable(true),
        m_dragging(false) {
}

ToolWindowManager::~ToolWindowManager() {
}

void ToolWindowManager::composeComponent() {
    Widget::composeComponent();

    m_rootLayout = this->layout();
    if(!m_rootLayout) {
        m_rootLayout = new Layout;
        m_rootLayout->setOrientation(Widget::Vertical);
        this->setLayout(m_rootLayout);
    }
}

void ToolWindowManager::update(const Vector2 &position) {
    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(m_dragging) {
            finishToolWindowDrag(position);
        }
        m_draggedToolWindow = nullptr;
        m_dropArea = nullptr;
        m_dragging = false;
        Input::mouseSetCursor(Input::CURSOR_ARROW);
    } else if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        m_draggedToolWindow = nullptr;
        m_dragging = false;
        m_dropArea = nullptr;

        for(AbstractToolWindowManagerArea *area : m_areas) {
            if(Widget *toolWindow = area->toolWindowAt(position)) {
                m_draggedToolWindow = toolWindow;
                m_dragStartPosition = position;
                break;
            }
        }
    } else if(m_draggedToolWindow && Input::isMouseButton(Input::MOUSE_LEFT)) {
        const float dx = position.x - m_dragStartPosition.x;
        const float dy = position.y - m_dragStartPosition.y;
        if(!m_dragging && (dx * dx + dy * dy) >= 36.0f) {
            m_dragging = true;
        }
        if(m_dragging) {
            updateDropTarget(position);
        }
    }

    Widget::update(position);
}

void ToolWindowManager::updateDropTarget(const Vector2 &position) {
    m_dropArea = nullptr;
    for(AbstractToolWindowManagerArea *area : m_areas) {
        const Vector4 bounds = area->clipRegion();
        if(position.x < bounds.x || position.x > bounds.x + bounds.z ||
           position.y < bounds.y || position.y > bounds.y + bounds.w ||
           bounds.z <= 0.0f || bounds.w <= 0.0f) {
            continue;
        }

        const float x = (position.x - bounds.x) / bounds.z;
        const float y = (position.y - bounds.y) / bounds.w;
        const float edgeDistance = 0.25f;
        m_dropArea = area;
        if(x < edgeDistance) {
            m_dropReference = ReferenceLeftOf;
            Input::mouseSetCursor(Input::CURSOR_HORSIZE);
        } else if(x > 1.0f - edgeDistance) {
            m_dropReference = ReferenceRightOf;
            Input::mouseSetCursor(Input::CURSOR_HORSIZE);
        } else if(y < edgeDistance) {
            m_dropReference = ReferenceBottomOf;
            Input::mouseSetCursor(Input::CURSOR_VERSIZE);
        } else if(y > 1.0f - edgeDistance) {
            m_dropReference = ReferenceTopOf;
            Input::mouseSetCursor(Input::CURSOR_VERSIZE);
        } else {
            m_dropReference = ReferenceAddTo;
            Input::mouseSetCursor(Input::CURSOR_HAND);
        }
        return;
    }
    Input::mouseSetCursor(Input::CURSOR_ARROW);
}

void ToolWindowManager::finishToolWindowDrag(const Vector2 &position) {
    updateDropTarget(position);
    if(!m_draggedToolWindow || !m_dropArea ||
       (areaFor(m_draggedToolWindow) == m_dropArea &&
        m_dropReference == ReferenceAddTo)) {
        return;
    }
    moveToolWindow(m_draggedToolWindow, m_dropReference, m_dropArea);
    if(AbstractToolWindowManagerArea *area = areaFor(m_draggedToolWindow)) {
        area->activateToolWindow(m_draggedToolWindow);
    }
}

void ToolWindowManager::activateToolWindow(Widget *toolWindow) {
    if(AbstractToolWindowManagerArea *area = areaFor(toolWindow)) {
        area->activateToolWindow(toolWindow);
    }
}

void ToolWindowManager::addToolWindow(Widget *toolWindow, AreaType area) {
    addToolWindows(std::vector<Widget *>{toolWindow}, area);
}

void ToolWindowManager::addToolWindow(Widget *toolWindow, ReferenceType reference,
                                       AbstractToolWindowManagerArea *area) {
    addToolWindows(std::vector<Widget *>{toolWindow}, reference, area);
}

void ToolWindowManager::addToolWindows(const std::vector<Widget *> &toolWindows, AreaType area) {
    if(area < LastUsedArea || area > NoArea) {
        aWarning() << "Invalid tool window area type";
        return;
    }
    if(area == NewFloatingArea) {
        aWarning() << "TODO: Native floating tool windows require Engine multi-window support";
        return;
    }

    std::vector<Widget *> added;
    for(Widget *toolWindow : toolWindows) {
        if(!toolWindow) {
            aWarning() << "Cannot add a null or uncomposed tool window";
            continue;
        }
        if(std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow) != m_toolWindows.end()) {
            aWarning() << "Tool window has already been added";
            continue;
        }
        m_toolWindows.push_back(toolWindow);
        added.push_back(toolWindow);
    }

    if(!added.empty()) {
        moveToolWindows(added, area);
    }
}

void ToolWindowManager::addToolWindows(const std::vector<Widget *> &toolWindows,
                                        ReferenceType reference,
                                        AbstractToolWindowManagerArea *area) {
    if(reference < ReferenceAddTo || reference > ReferenceBottomOf ||
       std::find(m_areas.begin(), m_areas.end(), area) == m_areas.end()) {
        aWarning() << "Invalid tool window area reference";
        return;
    }

    std::vector<Widget *> added;
    for(Widget *toolWindow : toolWindows) {
        if(!toolWindow) {
            aWarning() << "Cannot add a null or uncomposed tool window";
            continue;
        }
        if(std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow) != m_toolWindows.end()) {
            aWarning() << "Tool window has already been added";
            continue;
        }
        m_toolWindows.push_back(toolWindow);
        added.push_back(toolWindow);
    }

    if(!added.empty()) {
        moveToolWindows(added, reference, area);
    }
}

void ToolWindowManager::moveToolWindow(Widget *toolWindow, AreaType area) {
    moveToolWindows(std::vector<Widget *>{toolWindow}, area);
}

void ToolWindowManager::moveToolWindow(Widget *toolWindow, ReferenceType reference,
                                        AbstractToolWindowManagerArea *area) {
    moveToolWindows(std::vector<Widget *>{toolWindow}, reference, area);
}

void ToolWindowManager::moveToolWindows(const std::vector<Widget *> &toolWindows, AreaType area) {
    if(area < LastUsedArea || area > NoArea) {
        aWarning() << "Invalid tool window area type";
        return;
    }
    if(area == NewFloatingArea) {
        aWarning() << "TODO: Native floating tool windows require Engine multi-window support";
        return;
    }

    for(Widget *toolWindow : toolWindows) {
        if(std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow) == m_toolWindows.end()) {
            aWarning() << "Cannot move a tool window that is not managed";
            return;
        }
    }

    if(area == EmptySpaceArea) {
        for(AbstractToolWindowManagerArea *existing : m_areas) {
            for(Widget *current : existing->toolWindows()) {
                if(std::find(toolWindows.begin(), toolWindows.end(), current) == toolWindows.end()) {
                    aWarning() << "EmptySpaceArea is only available when the manager has no other panels";
                    return;
                }
            }
        }
    }

    for(Widget *toolWindow : toolWindows) {
        if(AbstractToolWindowManagerArea *areaForWindow = areaFor(toolWindow)) {
            areaForWindow->removeToolWindow(toolWindow);
        }
    }

    if(area == NoArea) {
        parkToolWindows(toolWindows);
        simplifyLayout();
        for(Widget *toolWindow : toolWindows) {
            emitSignal(_SIGNAL(toolWindowVisibilityChanged(Widget*)), toolWindow);
        }
        return;
    }

    AbstractToolWindowManagerArea *target = nullptr;
    if(area == LastUsedArea) {
        if(std::find(m_areas.begin(), m_areas.end(), m_lastUsedArea) != m_areas.end()) {
            target = m_lastUsedArea;
        } else if(!m_areas.empty()) {
            target = m_areas.front();
        } else {
            target = createAndSetupArea();
            if(target && m_rootLayout) {
                m_rootLayout->addWidget(target);
            }
        }
    } else if(area == EmptySpaceArea) {
        if(!m_areas.empty()) {
            target = m_areas.front();
        } else {
            target = createAndSetupArea();
            if(target && m_rootLayout) {
                m_rootLayout->addWidget(target);
            }
        }
    }

    if(!target) {
        aWarning() << "Unable to find or create a tool window area";
        parkToolWindows(toolWindows);
        simplifyLayout();
        return;
    }

    target->addToolWindows(toolWindows);
    m_lastUsedArea = target;
    simplifyLayout();

    for(Widget *toolWindow : toolWindows) {
        toolWindowVisibilityChanged(toolWindow);
    }
}

void ToolWindowManager::moveToolWindows(const std::vector<Widget *> &toolWindows,
                                        ReferenceType reference,
                                        AbstractToolWindowManagerArea *area) {
    if(reference < ReferenceAddTo || reference > ReferenceBottomOf ||
       std::find(m_areas.begin(), m_areas.end(), area) == m_areas.end()) {
        aWarning() << "Invalid tool window area reference";
        return;
    }

    for(Widget *toolWindow : toolWindows) {
        if(std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow) == m_toolWindows.end()) {
            aWarning() << "Cannot move a tool window that is not managed";
            return;
        }
    }

    for(Widget *toolWindow : toolWindows) {
        if(AbstractToolWindowManagerArea *areaForWindow = areaFor(toolWindow)) {
            areaForWindow->removeToolWindow(toolWindow);
        }
    }

    if(reference == ReferenceAddTo) {
        area->addToolWindows(toolWindows);
        m_lastUsedArea = area;
    } else {
        AbstractToolWindowManagerArea *added = createAndSetupArea();
        if(!added) {
            parkToolWindows(toolWindows);
            simplifyLayout();
            return;
        }
        if(!splitArea(area, added, reference)) {
            parkToolWindows(toolWindows);
            simplifyLayout();
            return;
        }
        added->addToolWindows(toolWindows);
        m_lastUsedArea = added;
    }

    simplifyLayout();
    for(Widget *toolWindow : toolWindows) {
        toolWindowVisibilityChanged(toolWindow);
    }
}

AbstractToolWindowManagerArea *ToolWindowManager::areaFor(Widget *toolWindow) const {
    if(!toolWindow) {
        return nullptr;
    }
    for(AbstractToolWindowManagerArea *area : m_areas) {
        const std::vector<Widget *> windows = area->toolWindows();
        if(std::find(windows.begin(), windows.end(), toolWindow) != windows.end()) {
            return area;
        }
    }
    return nullptr;
}

void ToolWindowManager::removeToolWindow(Widget *toolWindow) {
    auto it = std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow);
    if(it == m_toolWindows.end()) {
        return;
    }
    moveToolWindow(toolWindow, NoArea);
    if(toolWindow) {
        if(m_rootLayout) {
            m_rootLayout->removeWidget(toolWindow);
        }
        toolWindow->setParent(nullptr);
        toolWindow->setEnabled(true);
    }
    m_toolWindows.erase(std::remove(m_toolWindows.begin(), m_toolWindows.end(), toolWindow),
                        m_toolWindows.end());
}

std::vector<Widget *> ToolWindowManager::toolWindows() const {
    return m_toolWindows;
}

void ToolWindowManager::hideToolWindow(Widget *toolWindow) {
    if(std::find(m_toolWindows.begin(), m_toolWindows.end(), toolWindow) != m_toolWindows.end()) {
        moveToolWindow(toolWindow, NoArea);
    }
}

Variant ToolWindowManager::saveState() const {
    if(!m_rootLayout || m_rootLayout->count() == 0) {
        return Variant();
    }
    if(m_rootLayout->count() != 1) {
        aWarning() << "Cannot save tool window state while the layout has multiple roots";
        return Variant();
    }

    std::set<TString> names;
    for(Widget *toolWindow : m_toolWindows) {
        if(!toolWindow) {
            aWarning() << "Cannot save state with an uncomposed tool window";
            return Variant();
        }
        const TString name = toolWindow->name();
        if(name.isEmpty() || !names.insert(name).second) {
            aWarning() << "Tool window actor names must be unique and non-empty to save state";
            return Variant();
        }
    }

    VariantMap result;
    result[kStateFormat] = Variant(2);
    result[kRoot] = Variant(saveWidgetState(rootWidget()));
    return Variant(result);
}

bool ToolWindowManager::restoreState(const Variant &data) {
    if(!data.isValid()) {
        return false;
    }

    const VariantMap state = data.toMap();
    auto formatIt = state.find(kStateFormat);
    auto rootIt = state.find(kRoot);
    if(formatIt == state.end() || formatIt->second.toInt() != 2 || rootIt == state.end()) {
        aWarning() << "Tool window manager state format is not recognized";
        return false;
    }

    moveToolWindows(m_toolWindows, NoArea);
    Widget *root = restoreWidgetState(rootIt->second.toMap());
    if(!root) {
        simplifyLayout();
        return false;
    }
    m_rootLayout->addWidget(root);

    for(Widget *toolWindow : m_toolWindows) {
        //areaFor(toolWindow);
        toolWindowVisibilityChanged(toolWindow);
    }
    return true;
}

bool ToolWindowManager::tabsClosable() const {
    return m_tabsClosable;
}

void ToolWindowManager::setTabsClosable(bool enabled) {
    if(m_tabsClosable == enabled) {
        return;
    }
    m_tabsClosable = enabled;
    for(AbstractToolWindowManagerArea *area : m_areas) {
        area->setTabsClosable(enabled);
    }
}

void ToolWindowManager::toolWindowVisibilityChanged(Widget *toolWindow) {
    emitSignal(_SIGNAL(toolWindowVisibilityChanged(Widget*)), toolWindow);
}

void ToolWindowManager::currentToolWindowChanged(Widget *toolWindow) {
    emitSignal(_SIGNAL(currentToolWindowChanged(Widget*)), toolWindow);
}

void ToolWindowManager::onCurrentToolWindowChanged(Widget *toolWindow) {
    currentToolWindowChanged(toolWindow);
}

Splitter *ToolWindowManager::createSplitter() {
    Widget *splitterActor = Widget::createWidget<Splitter>(
                TString("toolWindowSplitter_%1").arg(TString::number(static_cast<int>(++m_splitterCounter))), this);
    if(!splitterActor) {
        return nullptr;
    }
    Splitter *splitter = dynamic_cast<Splitter *>(splitterActor);
    if(!splitter) {
        aError() << "Tool window splitter component is missing";
        splitterActor->deleteLater();
    }
    return splitter;
}

AbstractToolWindowManagerArea *ToolWindowManager::createArea() {
    Widget *areaActor = Widget::createWidget<ToolWindowManagerArea>(
                TString("toolWindowArea_%1").arg(TString::number(static_cast<int>(++m_areaCounter))), this);
    if(!areaActor) {
        return nullptr;
    }
    ToolWindowManagerArea *area = dynamic_cast<ToolWindowManagerArea *>(areaActor);
    if(!area) {
        aError() << "Tool window area component is missing";
        areaActor->deleteLater();
    }
    return area;
}

AbstractToolWindowManagerArea *ToolWindowManager::createAndSetupArea() {
    AbstractToolWindowManagerArea *area = createArea();
    if(!area) {
        aError() << "Unable to create tool window area";
        return nullptr;
    }
    area->setHorizontalPolicy(Widget::Expanding);
    area->setVerticalPolicy(Widget::Expanding);
    area->setManager(this);
    m_areas.push_back(area);
    connect(area, _SIGNAL(currentToolWindowChanged(Widget*)),
            this, _SLOT(onCurrentToolWindowChanged(Widget*)));
    return area;
}

Splitter *ToolWindowManager::createAndSetupSplitter() {
    Splitter *splitter = createSplitter();
    if(!splitter) {
        aError() << "Unable to create tool window splitter";
        return nullptr;
    }
    splitter->setHorizontalPolicy(Widget::Expanding);
    splitter->setVerticalPolicy(Widget::Expanding);
    splitter->setHandleWidth(6);
    splitter->setBackgroundColor(Vector4(0.35f, 0.35f, 0.35f, 1.0f));
    m_splitters.push_back(splitter);
    return splitter;
}

Widget *ToolWindowManager::rootWidget() const {
    if(!m_rootLayout || m_rootLayout->count() == 0) {
        return nullptr;
    }
    return m_rootLayout->widgetAt(0);
}

bool ToolWindowManager::splitArea(AbstractToolWindowManagerArea *target,
                                   AbstractToolWindowManagerArea *added,
                                   ReferenceType reference) {
    Widget *parent = target->parentWidget();
    if(!parent) {
        aWarning() << "Cannot split a detached tool window area";
        return false;
    }
    Layout *parentLayout = parent->layout();
    if(!parentLayout) {
        aWarning() << "Tool window area parent has no layout";
        return false;
    }

    const bool horizontal = reference == ReferenceLeftOf || reference == ReferenceRightOf;
    const bool before = reference == ReferenceLeftOf || reference == ReferenceTopOf;
    const int orientation = horizontal ? Widget::Horizontal : Widget::Vertical;
    Splitter *parentSplitter = dynamic_cast<Splitter *>(parent);
    const int targetIndex = parentLayout->indexOf(target);
    if(targetIndex < 0) {
        aWarning() << "Tool window area is missing from its parent layout";
        return false;
    }

    if(parentSplitter && parentSplitter->orentation() == orientation) {
        parentSplitter->insertWidget(before ? targetIndex : targetIndex + 1, added);
        equalizeSplitter(parentSplitter);
        return true;
    }

    Splitter *splitter = createAndSetupSplitter();
    if(!splitter) {
        return false;
    }
    splitter->setOrientation(orientation);

    target->setParent(nullptr);
    splitter->setParent(parent);
    parentLayout->insertWidget(targetIndex, splitter);

    if(before) {
        splitter->insertWidget(0, added);
        splitter->insertWidget(1, target);
    } else {
        splitter->insertWidget(0, target);
        splitter->insertWidget(1, added);
    }
    equalizeSplitter(splitter);
    return true;
}

void ToolWindowManager::equalizeSplitter(Splitter *splitter) {
    if(!splitter || splitter->count() < 2) {
        return;
    }

    const bool horizontal = splitter->orentation() == Widget::Horizontal;
    const Vector2 splitterSize = splitter->size();
    float available = horizontal ? splitterSize.x : splitterSize.y;
    if(available <= 0.0f) {
        available = 200.0f;
    }
    const float childSize = available / splitter->count();
    for(int i = 0; i < splitter->count() - 1; ++i) {
        Widget *child = splitter->widget(i);
        if(!child) {
            continue;
        }
        Vector2 size = child->size();
        if(horizontal) {
            size.x = childSize;
        } else {
            size.y = childSize;
        }
        child->setSize(size);
    }
}

void ToolWindowManager::parkToolWindows(const std::vector<Widget *> &toolWindows) {
    for(Widget *toolWindow : toolWindows) {
        if(toolWindow) {
            toolWindow->setParent(this);
            if(m_rootLayout) {
                m_rootLayout->removeWidget(toolWindow);
            }
            toolWindow->setEnabled(false);
        }
    }
}

void ToolWindowManager::simplifyLayout() {
    for(size_t i = 0; i < m_areas.size();) {
        AbstractToolWindowManagerArea *area = m_areas[i];
        if(!area->toolWindows().empty()) {
            ++i;
            continue;
        }

        if(m_lastUsedArea == area) {
            m_lastUsedArea = nullptr;
        }
        Widget *parent = area->parentWidget();
        Layout *parentLayout = parent ? parent->layout() : nullptr;
        if(parentLayout) {
            parentLayout->removeWidget(area);
        }
        if(area) {
            area->deleteLater();
        }
        m_areas.erase(m_areas.begin() + i);
    }

    bool changed = true;
    while(changed) {
        changed = false;
        for(size_t i = 0; i < m_splitters.size(); ++i) {
            Splitter *splitter = m_splitters[i];
            const int count = splitter->count();
            if(count > 1) {
                continue;
            }

            Widget *parent = splitter->parentWidget();
            Layout *parentLayout = parent ? parent->layout() : nullptr;
            const int index = parentLayout ? parentLayout->indexOf(splitter) : -1;
            if(!parentLayout || index < 0) {
                continue;
            }

            if(count == 1) {
                Widget *child = splitter->widget(0);
                if(child) {
                    parentLayout->insertWidget(index, child);
                }
            }
            parentLayout->removeWidget(splitter);
            if(splitter) {
                splitter->deleteLater();
            }
            m_splitters.erase(m_splitters.begin() + i);
            changed = true;
            break;
        }
    }
}

VariantMap ToolWindowManager::saveWidgetState(Widget *widget) const {
    VariantMap result;
    if(AbstractToolWindowManagerArea *area = dynamic_cast<AbstractToolWindowManagerArea *>(widget)) {
        result[kType] = Variant(kArea);
        VariantList names;
        for(Widget *toolWindow : area->toolWindows()) {
            if(toolWindow) {
                names.push_back(Variant(toolWindow->name()));
            }
        }
        result[kObjectNames] = Variant(names);
        result[kCustomData] = area->saveState();
        return result;
    }

    if(Splitter *splitter = dynamic_cast<Splitter *>(widget)) {
        result[kType] = Variant(kSplitter);
        result[kOrientation] = Variant(splitter->orentation());
        VariantList items;
        VariantList sizes;
        for(int i = 0; i < splitter->count(); ++i) {
            Widget *child = splitter->widget(i);
            if(!child) {
                continue;
            }
            items.push_back(Variant(saveWidgetState(child)));
            sizes.push_back(Variant(child->size()));
        }
        result[kItems] = Variant(items);
        result[kSizes] = Variant(sizes);
        return result;
    }

    aWarning() << "Cannot serialize an unknown tool window layout widget";
    return result;
}

Widget *ToolWindowManager::restoreWidgetState(const VariantMap &data) {
    auto typeIt = data.find(kType);
    if(typeIt == data.end()) {
        aWarning() << "Tool window layout node has no type";
        return nullptr;
    }

    if(typeIt->second.toString() == kArea) {
        return restoreAreaState(data);
    }

    if(typeIt->second.toString() != kSplitter) {
        aWarning() << "Unknown tool window layout node type";
        return nullptr;
    }

    auto itemsIt = data.find(kItems);
    if(itemsIt == data.end() || itemsIt->second.toList().empty()) {
        aWarning() << "Invalid empty splitter in saved tool window state";
        return nullptr;
    }

    auto orientationIt = data.find(kOrientation);
    int orientation = Widget::Horizontal;
    if(orientationIt != data.end()) {
        orientation = orientationIt->second.toInt();
        if(orientation != Widget::Horizontal && orientation != Widget::Vertical) {
            aWarning() << "Invalid splitter orientation in saved tool window state";
            return nullptr;
        }
    }

    Splitter *splitter = createAndSetupSplitter();
    if(!splitter) {
        return nullptr;
    }
    splitter->setOrientation(orientation);

    const VariantList items = itemsIt->second.toList();
    auto sizesIt = data.find(kSizes);
    const VariantList sizes = sizesIt != data.end() ? sizesIt->second.toList() : VariantList();
    auto sizeIt = sizes.begin();
    for(const Variant &item : items) {
        Widget *child = restoreWidgetState(item.toMap());
        if(child) {
            splitter->insertWidget(-1, child);
            if(sizeIt != sizes.end()) {
                child->setSize(sizeIt->toVector2());
            }
        }
        if(sizeIt != sizes.end()) {
            ++sizeIt;
        }
    }
    if(splitter->count() == 0) {
        if(splitter) {
            splitter->deleteLater();
        }
        m_splitters.erase(std::remove(m_splitters.begin(), m_splitters.end(), splitter),
                          m_splitters.end());
        return nullptr;
    }
    return splitter;
}

AbstractToolWindowManagerArea *ToolWindowManager::restoreAreaState(const VariantMap &data) {
    auto namesIt = data.find(kObjectNames);
    if(namesIt == data.end()) {
        aWarning() << "Tool window area has no tool window names";
        return nullptr;
    }

    std::vector<Widget *> windows;
    for(const Variant &nameValue : namesIt->second.toList()) {
        const TString name = nameValue.toString();
        auto it = std::find_if(m_toolWindows.begin(), m_toolWindows.end(),
                               [&name](Widget *window) {
            return window && window && window->name() == name;
        });
        if(it == m_toolWindows.end()) {
            aWarning() << "Tool window not found while restoring state:" << name;
            continue;
        }
        windows.push_back(*it);
    }
    if(windows.empty()) {
        return nullptr;
    }

    AbstractToolWindowManagerArea *area = createAndSetupArea();
    if(!area) {
        return nullptr;
    }
    area->addToolWindows(windows);
    auto stateIt = data.find(kCustomData);
    if(stateIt != data.end()) {
        area->restoreState(stateIt->second);
    }
    m_lastUsedArea = area;
    return area;
}
