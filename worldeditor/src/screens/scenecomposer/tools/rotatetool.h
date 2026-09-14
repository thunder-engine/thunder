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
#ifndef ROTATETOOL_H
#define ROTATETOOL_H

#include "selecttool.h"

#include <amath.h>

class RotateTool : public SelectTool {
public:
    explicit RotateTool(ObjectController *controller);

protected:
    void beginControl() override;

    void update(bool pivot, bool local, bool snap) override;

    QLineEdit *snapWidget() override;

    TString icon() const override;
    TString name() const override;

    TString toolTip() const override;
    TString shortcut() const override;

private:
    std::list<Vector3> m_eulers;
    std::list<Vector3> m_positions;
    std::list<Quaternion> m_quaternions;

};

#endif // ROTATETOOL_H
