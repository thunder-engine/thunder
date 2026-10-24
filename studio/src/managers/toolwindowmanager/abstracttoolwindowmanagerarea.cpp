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
#include "abstracttoolwindowmanagerarea.h"

#include "toolwindowmanager.h"

#include <layout.h>
#include <tabbar.h>
#include <tabwidget.h>
#include <engine.h>
#include <log.h>

AbstractToolWindowManagerArea::AbstractToolWindowManagerArea() :
        m_manager(nullptr),
        m_tabWidget(nullptr),
        m_tabBar(nullptr) {
}

ToolWindowManager *AbstractToolWindowManagerArea::manager() const {
    return m_manager;
}

void AbstractToolWindowManagerArea::setManager(ToolWindowManager *manager) {
    m_manager = manager;
    if(m_tabWidget) {
        m_tabWidget->setTabsClosable(manager && manager->tabsClosable());
    }
}

void AbstractToolWindowManagerArea::setTabsClosable(bool enabled) {
    if(m_tabWidget) {
        m_tabWidget->setTabsClosable(enabled);
    }
}

void AbstractToolWindowManagerArea::composeComponent() {
    Widget::composeComponent();

    Layout *layout = this->layout();
    if(!layout) {
        layout = new Layout;
        layout->setOrientation(Widget::Vertical);
        this->setLayout(layout);
    }

    Widget *tabActor = Widget::createWidget<TabWidget>("tabs", this);
    if(!tabActor) {
        aError() << "Unable to create tool window tab widget";
        return;
    }
    m_tabWidget = dynamic_cast<TabWidget *>(tabActor);
    if(!m_tabWidget) {
        aError() << "Tool window tab widget component is missing";
        return;
    }

    setSubWidget(m_tabWidget);
    m_tabWidget->setCloseOnRequest(false);
    Widget *tabTransform = m_tabWidget;
    tabTransform->setHorizontalPolicy(Widget::Expanding);
    tabTransform->setVerticalPolicy(Widget::Expanding);
    layout->addWidget(tabTransform);
    m_tabWidget->setTabsClosable(m_manager ? m_manager->tabsClosable() : true);

    m_tabBar = dynamic_cast<TabBar *>(m_tabWidget->subWidget("tabBar"));
    if(m_tabBar) {
        connect(m_tabBar, _SIGNAL(currentChanged(int)), this, _SLOT(onCurrentIndexChanged(int)));
    }
    connect(m_tabWidget, _SIGNAL(tabCloseRequested(int)), this, _SLOT(onTabCloseRequested(int)));
}

std::vector<Widget *> AbstractToolWindowManagerArea::toolWindows() const {
    std::vector<Widget *> result;
    if(!m_tabWidget) {
        return result;
    }

    for(int i = 0; i < m_tabWidget->count(); ++i) {
        if(Widget *toolWindow = m_tabWidget->tabContent(i)) {
            result.push_back(toolWindow);
        }
    }
    return result;
}

Widget *AbstractToolWindowManagerArea::toolWindowAt(const Vector2 &position) const {
    if(!m_tabWidget || !m_tabBar) {
        return nullptr;
    }

    Widget *tabBarWidget = m_tabBar;
    if(!tabBarWidget->isHovered(position)) {
        return nullptr;
    }

    for(int i = 0; i < m_tabWidget->count(); ++i) {
        Button *tab = m_tabBar->tabButton(i);
        if(tab && tab->isHovered(position)) {
            return m_tabWidget->tabContent(i);
        }
    }
    return nullptr;
}

void AbstractToolWindowManagerArea::activateToolWindow(Widget *toolWindow) {
    if(!m_tabWidget || !toolWindow) {
        return;
    }
    for(int i = 0; i < m_tabWidget->count(); ++i) {
        if(m_tabWidget->tabContent(i) == toolWindow) {
            const int previousIndex = m_tabWidget->currentIndex();
            m_tabWidget->setCurrentIndex(i);
            if(previousIndex != i) {
                currentToolWindowChanged(toolWindow);
            }
            return;
        }
    }
}

void AbstractToolWindowManagerArea::addToolWindows(const std::vector<Widget *> &toolWindows) {
    if(!m_tabWidget) {
        aWarning() << "Tool window area has not been composed";
        return;
    }
    const int previousIndex = m_tabWidget->currentIndex();
    for(Widget *toolWindow : toolWindows) {
        if(toolWindow && toolWindow) {
            m_tabWidget->addTab(toolWindow->name(), toolWindow);
        }
    }
    if(previousIndex != m_tabWidget->currentIndex()) {
        currentToolWindowChanged(m_tabWidget->tabContent(m_tabWidget->currentIndex()));
    }
}

void AbstractToolWindowManagerArea::removeToolWindow(Widget *toolWindow) {
    if(!m_tabWidget || !toolWindow) {
        return;
    }
    for(int i = 0; i < m_tabWidget->count(); ++i) {
        if(m_tabWidget->tabContent(i) == toolWindow) {
            Widget *previousCurrent = m_tabWidget->tabContent(m_tabWidget->currentIndex());
            m_tabWidget->takeTab(i);
            Widget *current = m_tabWidget->tabContent(m_tabWidget->currentIndex());
            if(previousCurrent != current) {
                currentToolWindowChanged(current);
            }
            return;
        }
    }
}

Variant AbstractToolWindowManagerArea::saveState() const {
    VariantMap result;
    result[TString("currentIndex")] = Variant(m_tabWidget ? m_tabWidget->currentIndex() : -1);
    return Variant(result);
}

void AbstractToolWindowManagerArea::restoreState(const Variant &state) {
    if(m_tabWidget) {
        const VariantMap map = state.toMap();
        auto it = map.find(TString("currentIndex"));
        if(it != map.end()) {
            Widget *previousCurrent = m_tabWidget->tabContent(m_tabWidget->currentIndex());
            m_tabWidget->setCurrentIndex(it->second.toInt());
            Widget *current = m_tabWidget->tabContent(m_tabWidget->currentIndex());
            if(previousCurrent != current) {
                currentToolWindowChanged(current);
            }
        }
    }
}

void AbstractToolWindowManagerArea::onCurrentIndexChanged(int index) {
    if(m_tabWidget && index >= 0) {
        currentToolWindowChanged(m_tabWidget->tabContent(index));
    }
}

void AbstractToolWindowManagerArea::onTabCloseRequested(int index) {
    if(m_tabWidget && m_manager) {
        if(Widget *toolWindow = m_tabWidget->tabContent(index)) {
            m_manager->hideToolWindow(toolWindow);
        }
    }
}

void AbstractToolWindowManagerArea::currentToolWindowChanged(Widget *toolWindow) {
    emitSignal(_SIGNAL(currentToolWindowChanged(Widget *)), Variant::fromValue(toolWindow));
}
