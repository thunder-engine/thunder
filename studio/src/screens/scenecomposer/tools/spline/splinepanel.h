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

#include <components/widget.h>

class SplineTool;
class LineEdit;
class Button;

class SplinePanel : public Widget {
    A_OBJECT(SplinePanel, Widget, Editor)

    A_METHODS(
        A_SLOT(SplinePanel::onEditFinished),
        A_SLOT(SplinePanel::onBreakPointClicked),
        A_SLOT(SplinePanel::onAddPointClicked),
        A_SLOT(SplinePanel::onDeletePointClicked)
    )
    A_NOENUMS()
    A_NOPROPERTIES()

public:
    SplinePanel();

    void setTool(SplineTool *tool);

    void updateValues();

protected:
    void composeComponent() override;

private:
    void onEditFinished();
    void onBreakPointClicked();
    void onAddPointClicked();
    void onDeletePointClicked();

private:
    SplineTool *m_tool;
    LineEdit *m_xEdit;
    LineEdit *m_yEdit;
    LineEdit *m_zEdit;
    Button *m_breakPoint;

};

#endif // SPLINEPANEL_H
