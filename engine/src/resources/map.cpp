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

#include "resources/map.h"

#include "components/scene.h"

/*!
    \class Map
    \brief A container which holds deserialized Scene object.
    \inmodule Resources
*/

Map::Map() :
        m_scene(nullptr) {

}

Map::~Map() {
    if(m_scene) {
        m_scene->setMap(nullptr);
    }
}

/*!
    Returns a scene which can be added to the scene graph.
*/
Scene *Map::scene() const {
    if(m_scene == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            m_scene = dynamic_cast<Scene *>(children.front());
            m_scene->setMap(const_cast<Map *>(this));
        }
    }
    return m_scene;
}
/*!
    \internal
*/
void Map::setScene(Scene *scene) {
    m_scene = scene;
    if(m_scene) {
        m_scene->setMap(this);
    }
}
