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
#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "editor/editortool.h"

class ObjectController;

class QLineEdit;

class SelectTool : public EditorTool {
public:
    struct Select {
        bool operator==(const Select &left) const {
            return (uuid == left.uuid);
        }

        uint32_t uuid = 0;
        Actor *object = nullptr;
    };

    typedef QList<Select> SelectList;

public:
    explicit
    SelectTool(ObjectController *controller);

    virtual QLineEdit *snapWidget();

    float snap() const;
    void setSnap(float snap);

protected:
    void update(bool center, bool local, bool snap) override;

    void beginControl() override;
    void endControl() override;
    void cancelControl() override;

    TString icon() const override;
    TString name() const override;

    TString component() const override;

    Vector3 objectPosition();
    AABBox objectBound();

protected:
    Vector3 m_world;
    Vector3 m_savedWorld;
    Vector3 m_position;

    ObjectController *m_controller;

    QLineEdit *m_snapEditor;

    float m_snap;

};

#endif // SELECTTOOL_H
