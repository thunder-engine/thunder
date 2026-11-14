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
#include "components/topmost.h"

#include "components/canvas.h"

#include <log.h>

#include <algorithm>

TopMost::TopMost() = default;

TopMost::~TopMost() {
    if(Canvas *canvas = this->canvas()) {
        canvas->hideTopMost(this);
    }
}

TopMost::Level TopMost::level() const {
    return m_level;
}

void TopMost::setLevel(Level level) {
    if(m_level != level) {
        m_level = level;
        repaint();
    }
}

bool TopMost::show(Widget *anchor, const Vector2 &offset) {
    if(!anchor) {
        aWarning() << "Unable to show a topmost widget without an anchor.";
        return false;
    }

    Canvas *canvas = anchor->canvas();
    if(!canvas) {
        aWarning() << "Unable to show a topmost widget without a canvas.";
        return false;
    }
    Canvas *root = canvas;
    for(Widget *parent = canvas->parentWidget(); parent; parent = parent->parentWidget()) {
        if(Canvas *parentCanvas = dynamic_cast<Canvas *>(parent)) {
            root = parentCanvas;
        }
    }

    const Vector3 anchorPosition = root->worldTransform().inverse() * anchor->worldPosition();
    const float rootScale = root->worldScale();
    const float anchorScale = rootScale != 0.0f ? anchor->worldScale() / rootScale : anchor->worldScale();
    const Vector2 anchorSize(anchor->size() * anchorScale);
    const float widgetScale = rootScale != 0.0f ? worldScale() / rootScale : worldScale();
    const Vector2 widgetSize(size() * widgetScale);
    const Vector2 canvasSize(root->size());
    const float gap = 4.0f;
    const float anchorLeft = anchorPosition.x;
    const float anchorBottom = anchorPosition.y;
    const float anchorRight = anchorLeft + anchorSize.x;
    const float anchorTop = anchorBottom + anchorSize.y;
    const float below = anchorBottom - gap;
    const float above = canvasSize.y - anchorTop - gap;
    const float right = canvasSize.x - anchorRight - gap;
    const float left = anchorLeft - gap;

    enum Placement { Below, Above, Right, Left };
    Placement placement;
    if(offset.y < 0.0f && below >= widgetSize.y) {
        placement = Below;
    } else if(offset.y > 0.0f && above >= widgetSize.y) {
        placement = Above;
    } else if(below >= widgetSize.y) {
        placement = Below;
    } else if(above >= widgetSize.y) {
        placement = Above;
    } else if(offset.x > 0.0f && right >= widgetSize.x) {
        placement = Right;
    } else if(offset.x < 0.0f && left >= widgetSize.x) {
        placement = Left;
    } else if(right >= widgetSize.x) {
        placement = Right;
    } else if(left >= widgetSize.x) {
        placement = Left;
    } else {
        aWarning() << "Unable to place a topmost widget next to its anchor.";
        return false;
    }

    if(widgetSize.x > canvasSize.x || widgetSize.y > canvasSize.y) {
        aWarning() << "Unable to place a topmost widget larger than its canvas.";
        return false;
    }

    float x = anchorLeft;
    float top = anchorTop;
    switch(placement) {
        case Below:
            x += offset.x;
            top = below;
            break;
        case Above:
            x += offset.x;
            top = anchorTop + gap + widgetSize.y;
            break;
        case Right:
            x = anchorRight + gap;
            top = anchorTop + offset.y;
            break;
        case Left:
            x = anchorLeft - gap - widgetSize.x;
            top = anchorTop + offset.y;
            break;
    }
    x = std::max(0.0f, std::min(x, canvasSize.x - widgetSize.x));
    top = std::max(widgetSize.y, std::min(top, canvasSize.y));

    if(Canvas *previousRoot = this->canvas()) {
        previousRoot->hideTopMost(this);
    }
    setParent(root);
    setAnchors(Vector2(0.0f), Vector2(0.0f));
    setPivot(Vector2(0.0f, 1.0f));

    setPosition(Vector3(x, top, 0.0f));
    setVisible(true);
    root->showTopMost(this);
    return true;
}

void TopMost::hide() {
    if(Canvas *root = canvas()) {
        root->hideTopMost(this);
    }
    setVisible(false);
}

void TopMost::draw() {
    Canvas *root = canvas();
    if(root && root->isDrawingTopMost()) {
        Frame::draw();
    }
}
