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
#include "splinepanel.h"

#include "splinetool.h"

#include <components/actor.h>
#include <components/spline.h>

#include <button.h>
#include <label.h>
#include <layout.h>
#include <lineedit.h>

#include <iomanip>
#include <sstream>
#include <string>

namespace {
    TString formatCoordinate(float value) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(4) << value;
        std::string result(stream.str());
        while(!result.empty() && result.back() == '0') {
            result.pop_back();
        }
        if(!result.empty() && result.back() == '.') {
            result.pop_back();
        }
        return result;
    }
}

SplinePanel::SplinePanel() :
        m_tool(nullptr),
        m_xEdit(nullptr),
        m_yEdit(nullptr),
        m_zEdit(nullptr),
        m_breakPoint(nullptr) {
}

void SplinePanel::composeComponent() {
    Widget::composeComponent();

    Widget *root = this;
    if(root) {
        Layout *layout = new Layout;
        layout->setOrientation(Widget::Horizontal);
        layout->setSpacing(5);
        root->setLayout(layout);
        root->setSize(Vector2(458.0f, 24.0f));
        root->setHorizontalPolicy(Widget::Preferred);
        root->setVerticalPolicy(Widget::Fixed);
    }

    auto addLabel = [this](const TString &name, const TString &text, float width) {
        Widget *labelActor = Widget::createWidget<Label>(name, this);
        Label *label = labelActor ? dynamic_cast<Label *>(labelActor) : nullptr;
        if(label) {
            label->setText(text);
            label->setSize(Vector2(width, 22.0f));
            this->layout()->addWidget(label);
        }
        return label;
    };
    auto addInput = [this](const TString &name) {
        Widget *inputActor = Widget::createWidget<LineEdit>(name, this);
        LineEdit *input = inputActor ? dynamic_cast<LineEdit *>(inputActor) : nullptr;
        if(input) {
            input->setSize(Vector2(75.0f, 22.0f));
            this->layout()->addWidget(input);
            connect(input, _SIGNAL(editingFinished()), this, _SLOT(onEditFinished()));
        }
        return input;
    };

    addLabel("title", "Spline tool", 80.0f);
    addLabel("x-label", "X:", 14.0f);
    m_xEdit = addInput("x");
    addLabel("y-label", "Y:", 14.0f);
    m_yEdit = addInput("y");
    addLabel("z-label", "Z:", 14.0f);
    m_zEdit = addInput("z");

    Widget *breakActor = Widget::createWidget<Button>("break-point", this);
    m_breakPoint = breakActor ? dynamic_cast<Button *>(breakActor) : nullptr;
    if(m_breakPoint) {
        m_breakPoint->setText("Break");
        m_breakPoint->setCheckable(true);
        m_breakPoint->setSize(Vector2(48.0f, 22.0f));
        this->layout()->addWidget(m_breakPoint);
        connect(m_breakPoint, _SIGNAL(clicked()), this, _SLOT(onBreakPointClicked()));
    }

    auto addButton = [this](const TString &name, const TString &text, const char *slot) {
        Widget *buttonActor = Widget::createWidget<Button>(name, this);
        Button *button = buttonActor ? dynamic_cast<Button *>(buttonActor) : nullptr;
        if(button) {
            button->setText(text);
            button->setSize(Vector2(40.0f, 22.0f));
            this->layout()->addWidget(button);
            connect(button, _SIGNAL(clicked()), this, slot);
        }
    };
    addButton("add-point", "Add", _SLOT(onAddPointClicked()));
    addButton("delete-point", "Delete", _SLOT(onDeletePointClicked()));
}

void SplinePanel::setTool(SplineTool *tool) {
    m_tool = tool;
}

void SplinePanel::updateValues() {
    if(!m_tool || !m_xEdit || !m_yEdit || !m_zEdit || !m_breakPoint) {
        return;
    }
    int index = m_tool->pointIndex();
    Spline *spline = m_tool->spline();
    if(spline && index > -1) {
        Spline::Point point = spline->point(index);
        m_xEdit->setText(formatCoordinate(point.position.x));
        m_yEdit->setText(formatCoordinate(point.position.y));
        m_zEdit->setText(formatCoordinate(point.position.z));
        m_breakPoint->setChecked(point.breaked);
    }
}

void SplinePanel::onEditFinished() {
    if(!m_tool || !m_xEdit || !m_yEdit || !m_zEdit || !m_breakPoint) {
        return;
    }
    int index = m_tool->pointIndex();
    Spline *spline = m_tool->spline();
    if(spline && index > -1) {
        Spline::Point point = spline->point(index);
        Vector3 value(m_xEdit->text().toFloat(), m_yEdit->text().toFloat(), m_zEdit->text().toFloat());

        bool checked = m_breakPoint->isChecked();
        if(point.position != value || point.breaked != checked) {
            Vector3 delta(value - point.position);
            point.position += delta;
            point.tangentIn += delta;
            point.tangentOut += delta;
            if(!checked && point.breaked != checked) {
                delta = point.tangentOut - point.position;
                point.tangentIn = point.position - delta;
            }
            point.breaked = checked;
            m_tool->controller()->undoRedo()->push(new ChangeSplinePoint(point, m_tool));
        }
    }
}

void SplinePanel::onBreakPointClicked() {
    onEditFinished();
}

void SplinePanel::onAddPointClicked() {
    if(m_tool) {
        m_tool->controller()->undoRedo()->push(new InsertSplinePoint(0.5f, m_tool));
    }
}

void SplinePanel::onDeletePointClicked() {
    if(m_tool) {
        m_tool->controller()->undoRedo()->push(new DeleteSplinePoint(m_tool));
    }
}
