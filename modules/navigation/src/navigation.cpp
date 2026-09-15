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
#include "navigation.h"

#include "navigationsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "navigationpanel.h"
#include "property/agenttypeedit.h"

Module *moduleCreate(Engine *engine) {
    return new Navigation(engine);
}
#endif

static const char *meta = \
    "{"
    "   \"module\": \"Navigation\","
    "   \"version\": \"1.0\","
    "   \"description\": \"AI Navigation Module\","
    "   \"author\": \"Evgeniy Prikazchikov\","
    "   \"dependencies\": {"
    "       \"Bullet\": \"module\""
    "   },"
    "   \"objects\": {"
    "       \"NavigationSystem\": \"system\","
    "       \"NavigationPanel\": \"gadget\","
    "       \"AgentTypeEdit\": \"property\""
    "   },"
    "   \"components\": ["
    "       \"NavMeshAgent\","
    "       \"NavMeshLink\","
    "       \"NavMeshObstacle\","
    "       \"NavMeshSurface\""
    "   ]"
    "}";

/*!
    \module Navigation

    \title Navigation Module

    \brief Provides navigation and pathfinding functionality for Thunder Engine.
*/

Navigation::Navigation(Engine *engine) :
        Module(engine),
        m_system(nullptr),
        m_panel(nullptr) {
}

Navigation::~Navigation() {
    delete m_system;
#ifdef SHARED_DEFINE
    delete m_panel;
#endif
}

const char *Navigation::metaInfo() const {
    return meta;
}

void *Navigation::getObject(const char *name) {
    if(strcmp(name, "NavigationSystem") == 0) {
        if(m_system == nullptr) {
            m_system = new NavigationSystem();
        }
        return m_system;
    }
#ifdef SHARED_DEFINE
    if(strcmp(name, "NavigationPanel") == 0) {
        if(m_panel == nullptr) {
            m_panel = new NavigationPanel();
        }
        return m_panel;
    }
    if(strcmp(name, "AgentTypeEdit") == 0) {
        return new AgentTypeEdit();
    }
#endif
    return nullptr;
}
