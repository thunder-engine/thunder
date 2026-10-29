#include "spritetool.h"

#include <gizmos.h>
#include <input.h>

#include <material.h>

#include <actor.h>
#include <transform.h>
#include <camera.h>
#include <spriterender.h>

#include <viewport/handles.h>

#include "../spritecontroller.h"
#include "../actions/selectsprite.h"
#include "../actions/updatesprite.h"
#include "../actions/createsprite.h"
#include "../actions/destroysprite.h"

SpriteTool::SpriteTool(SpriteController *controller) :
        m_controller(controller),
        m_settings(nullptr),
        m_borderAxes(0),
        m_useBorder(false) {

    m_item.setController(m_controller);
}

void SpriteTool::setSettings(TextureImportSettings *settings) {
    m_settings = settings;

    m_item.setSettings(m_settings);
}

Object::ObjectList SpriteTool::selected() {
    return {&m_item};
}

void SpriteTool::beginControl() {
    m_savedPoint = m_currentPoint;

    m_savedElements = m_settings->elements();

    m_useBorder =
            (m_borderAxes == (Handles::LEFT | Handles::TOP)) ||
            (m_borderAxes == (Handles::LEFT | Handles::BOTTOM)) ||
            (m_borderAxes == (Handles::RIGHT | Handles::TOP)) ||
            (m_borderAxes == (Handles::RIGHT | Handles::BOTTOM)) ||
            m_borderAxes == Handles::LEFT || m_borderAxes == Handles::RIGHT ||
            m_borderAxes == Handles::TOP || m_borderAxes == Handles::BOTTOM;
}

void SpriteTool::cancelControl() {
    for(auto &it : m_settings->elements()) {
        auto &saved = m_savedElements[it.first];

        it.second.borderMin = saved.borderMin;
        it.second.borderMax = saved.borderMax;

        it.second.min = saved.min;
        it.second.max = saved.max;

        it.second.pivot = saved.pivot;
    }
}

void SpriteTool::update(bool pivot, bool local, bool snap) {
    bool isDrag = m_controller->isDrag();
    Input::CursorShape shape = Input::CURSOR_ARROW;

    const float height = static_cast<float>(m_controller->height());

    m_currentPoint = m_controller->world();

    TextureImportSettings::Element element;

    TString selected(m_controller->selectedElement());
    for(auto &it : m_settings->elements()) {
        TextureImportSettings::Element ed = it.second;
        ed.min.y = height - it.second.max.y;
        ed.max.y = height - it.second.min.y;

        AABBox rect;
        rect.setBox(Vector3(ed.min, 0.0f), Vector3(ed.max, 0.0f));

        if(m_controller->isSelected(it.first)) {
            Vector3 tl0(ed.min.x, ed.max.y - ed.borderMax.y, 0.0f);
            Vector3 tr0(ed.max.x, ed.max.y - ed.borderMax.y, 0.0f);
            Vector3 bl0(ed.min.x, ed.min.y + ed.borderMin.y, 0.0f);
            Vector3 br0(ed.max.x, ed.min.y + ed.borderMin.y, 0.0f);

            Vector3 tl1(ed.min.x + ed.borderMin.x, ed.max.y, 0.0f);
            Vector3 tr1(ed.max.x - ed.borderMax.x, ed.max.y, 0.0f);
            Vector3 bl1(ed.min.x + ed.borderMin.x, ed.min.y, 0.0f);
            Vector3 br1(ed.max.x - ed.borderMax.x, ed.min.y, 0.0f);

            Gizmos::drawLines({tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0,1,2,3,4,5,6,7}, Handles::s_yColor);

            AABBox sub;
            sub.setBox(Vector3(ed.min.x + ed.borderMin.x, ed.min.y + ed.borderMin.y, 0.0f),
                       Vector3(ed.max.x - ed.borderMax.x, ed.max.y - ed.borderMax.y, 0.0f));

            int axis;

            if(!isDrag) {
                m_borderAxes = 0;
            }

            Handles::s_Color = Handles::s_yColor;
            Handles::rectTool(sub.center, sub.extent * 2.0f, axis, true, isDrag);

            if(!isDrag) {
                m_borderAxes = Handles::s_Axes;
            }

            Handles::s_Color = Handles::s_zColor;
            m_currentPoint = Handles::rectTool(rect.center, rect.extent * 2.0f, axis, false, isDrag);
            Handles::s_Color = Handles::s_Normal;

            uint8_t axes = Handles::s_Axes;
            if(m_useBorder) {
                axes = m_borderAxes;
            }

            if(isDrag) {
                Vector3 delta(m_currentPoint - m_savedPoint);

                Vector2 min(ed.min);
                Vector2 max(ed.max);

                if(m_useBorder) {
                    min = Vector2(ed.borderMin.x, ed.borderMin.y);
                    max = Vector2(ed.borderMax.x, ed.borderMax.y);
                }

                if(axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
                    min += delta;
                    max += delta;
                } else if(axes == (Handles::TOP | Handles::RIGHT)) {
                    max += m_useBorder ? -delta : delta;
                } else if(axes == (Handles::TOP | Handles::LEFT)) {
                    min.x += delta.x;
                    max.y += m_useBorder ? -delta.y : delta.y;
                } else if(axes == (Handles::BOTTOM | Handles::RIGHT)) {
                    max.x += m_useBorder ? -delta.x : delta.x;
                    min.y += delta.y;
                } else if(axes == (Handles::BOTTOM | Handles::LEFT)) {
                    min += delta;
                } else if(axes == Handles::TOP) {
                    max.y += m_useBorder ? -delta.y : delta.y;
                } else if(axes == Handles::BOTTOM) {
                    min.y += delta.y;
                } else if(axes == Handles::LEFT) {
                    min.x += delta.x;
                } else if(axes == Handles::RIGHT) {
                    max.x += m_useBorder ? -delta.x : delta.x;
                }

                if(m_useBorder) {
                    Vector2 size(ed.max - ed.min);

                    ed.borderMin.x = CLAMP(min.x, 0.0f, size.x);
                    ed.borderMin.y = CLAMP(min.y, 0.0f, size.y - max.y);

                    ed.borderMax.x = CLAMP(max.x, 0.0f, size.x - min.x);
                    ed.borderMax.y = CLAMP(max.y, 0.0f, size.y);

                    it.second.borderMin.x = ed.borderMin.x;
                    it.second.borderMin.y = ed.borderMin.y;
                    it.second.borderMax.x = ed.borderMax.x;
                    it.second.borderMax.y = ed.borderMax.y;
                } else {
                    ed.min.x = MAX(min.x, 0.0f);
                    ed.min.y = MAX(min.y, 0.0f);
                    ed.max.x = MIN(max.x, m_controller->width());
                    ed.max.y = MIN(max.y, height);

                    it.second.min.x = ed.min.x;
                    it.second.max.x = ed.max.x;
                    it.second.min.y = height - ed.max.y;
                    it.second.max.y = height - ed.min.y;
                }

                m_controller->updated();

                m_savedPoint = m_currentPoint;
            }

            if(axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
                if(m_borderAxes != 0 && m_borderAxes != (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
                    axes = m_borderAxes;
                }
                shape = Input::CURSOR_ALLSIZE;
            }

            if(axes == (Handles::TOP | Handles::RIGHT)) {
                shape = Input::CURSOR_BDIAGSIZE;
            } else if(axes == (Handles::TOP | Handles::LEFT)) {
                shape = Input::CURSOR_FDIAGSIZE;
            } else if(axes == (Handles::BOTTOM | Handles::RIGHT)) {
                shape = Input::CURSOR_FDIAGSIZE;
            } else if(axes == (Handles::BOTTOM | Handles::LEFT)) {
                shape = Input::CURSOR_BDIAGSIZE;
            } else if(axes == Handles::TOP || axes == Handles::BOTTOM) {
                shape = Input::CURSOR_VERSIZE;
            } else if(axes == Handles::LEFT || axes == Handles::RIGHT) {
                shape = Input::CURSOR_HORSIZE;
            }

            element = it.second;
        } else {
            Gizmos::drawWireRectangle(rect.center, Vector2(rect.extent.x * 2.0f, rect.extent.y * 2.0f), Handles::s_xColor);
        }
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        m_currentPoint = m_startPoint = m_controller->world();

        if(Handles::s_Axes == 0) {
            TString key;

            Vector3 world = m_controller->world();
            for(auto &it : m_settings->elements()) {
                float yMin = height - it.second.max.y;
                float yMax = height - it.second.min.y;
                if(it.second.min.x < world.x && it.second.max.x > world.x &&
                    yMin < world.y && yMax > world.y) {
                    key = it.first;
                    break;
                }
            }

            if(key.isEmpty()) {
                m_controller->selectionChanged();
                m_controller->undoRedo()->push(new SelectSprite("", m_controller));
            } else {
                m_item.setKey(key);
                m_controller->selectionChanged();
                m_controller->undoRedo()->push(new SelectSprite(key, m_controller));
            }
        } else if(!isDrag) {
            m_controller->setDrag(true);
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(isDrag && !selected.isEmpty()) {
            cancelControl();
            m_controller->undoRedo()->push(new UpdateSprite(element, m_controller));
        }

        m_useBorder = false;
        m_controller->setDrag(false);

        if(selected.isEmpty() && m_currentPoint != m_startPoint) {
            TextureImportSettings::Element element;

            float xmin = MIN(m_startPoint.x, m_currentPoint.x);
            float xmax = MAX(m_startPoint.x, m_currentPoint.x);
            float ymin = MIN(m_startPoint.y, m_currentPoint.y);
            float ymax = MAX(m_startPoint.y, m_currentPoint.y);

            element.min = Vector2(xmin, height - ymax);
            element.max = Vector2(xmax, height - ymin);

            m_controller->undoRedo()->push(new CreateSprite(element, m_controller));
            m_startPoint = m_currentPoint;
        }
    }

    if(selected.isEmpty() && Input::isMouseButton(Input::MOUSE_LEFT) && m_currentPoint != m_startPoint) {
        AABBox bb;
        bb.setBox(m_startPoint, m_currentPoint);
        Gizmos::drawWireRectangle(bb.center, Vector2(bb.extent.x * 2.0f, bb.extent.y * 2.0f), Handles::s_yColor);
    }

    if(isDrag && Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        cancelControl();
        m_useBorder = false;
        m_controller->setDrag(false);
    }

    if(!selected.isEmpty() && Input::isKeyDown(Input::KEY_DELETE)) {
        m_controller->undoRedo()->push(new DestroySprite(m_controller));
    }

    Input::mouseSetCursor(shape);
}

TString SpriteTool::icon() const {
    return ":/Images/editor/Transform.png";
}

TString SpriteTool::name() const {
    return "Resize";
}

TString SpriteTool::component() const {
    return SpriteRender::metaClass()->name();
}
