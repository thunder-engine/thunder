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

}

void SpriteTool::setSettings(TextureImportSettings *settings) {
    m_settings = settings;
}

void SpriteTool::beginControl() {
    m_savedPoint = m_currentPoint;

    m_savedElements.clear();
    for(auto &it : m_settings->elements()) {
        m_savedElements[it.first] = ResourceSystem::saveResourceProperties(it.second);
    }

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

        ResourceSystem::loadResourceProperties(it.second, saved.toMap());
    }
}

void SpriteTool::update(bool pivot, bool local, bool snap) {
    bool isDrag = m_controller->isDrag();
    Input::CursorShape shape = Input::CURSOR_ARROW;

    const uint32_t height = static_cast<float>(m_controller->height());

    m_currentPoint = m_controller->world();

    VariantMap elementData;

    TString selected(m_controller->selectedElement());
    for(auto &it : m_settings->elements()) {
        Sprite *ed = it.second;
        Vector4 bounds(ed->bounds());
        Vector4 border(ed->border());

        Vector2 min(bounds.x, bounds.y);
        Vector2 max(bounds.z, bounds.w);
        Vector2 edmin(min);
        Vector2 edmax(max);
        Vector2 edborderMin(border.x, border.y);
        Vector2 edborderMax(border.z, border.w);

        edmin.y = height - max.y;
        edmax.y = height - min.y;

        AABBox rect;
        rect.setBox(Vector3(edmin, 0.0f), Vector3(edmax, 0.0f));

        if(m_controller->isSelected(it.first)) {
            Vector3 tl0(edmin.x, edmax.y - edborderMax.y, 0.0f);
            Vector3 tr0(edmax.x, edmax.y - edborderMax.y, 0.0f);
            Vector3 bl0(edmin.x, edmin.y + edborderMin.y, 0.0f);
            Vector3 br0(edmax.x, edmin.y + edborderMin.y, 0.0f);

            Vector3 tl1(edmin.x + edborderMin.x, edmax.y, 0.0f);
            Vector3 tr1(edmax.x - edborderMax.x, edmax.y, 0.0f);
            Vector3 bl1(edmin.x + edborderMin.x, edmin.y, 0.0f);
            Vector3 br1(edmax.x - edborderMax.x, edmin.y, 0.0f);

            Gizmos::drawLines({tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0,1,2,3,4,5,6,7}, Handles::s_yColor);

            AABBox sub;
            sub.setBox(Vector3(edmin.x + edborderMin.x, edmin.y + edborderMin.y, 0.0f),
                       Vector3(edmax.x - edborderMax.x, edmax.y - edborderMax.y, 0.0f));

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

                Vector2 min(edmin);
                Vector2 max(edmax);

                if(m_useBorder) {
                    min = Vector2(edborderMin.x, edborderMin.y);
                    max = Vector2(edborderMax.x, edborderMax.y);
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
                    Vector2 size(edmax - edmin);

                    edborderMin.x = CLAMP(min.x, 0.0f, size.x);
                    edborderMin.y = CLAMP(min.y, 0.0f, size.y - max.y);

                    edborderMax.x = CLAMP(max.x, 0.0f, size.x - min.x);
                    edborderMax.y = CLAMP(max.y, 0.0f, size.y);

                    ed->setBorder(Vector4(edborderMin, edborderMax));
                } else {
                    edmin.x = MAX(min.x, 0.0f);
                    edmin.y = MAX(min.y, 0.0f);
                    edmax.x = MIN(max.x, m_controller->width());
                    edmax.y = MIN(max.y, height);

                    ed->setBounds(Vector4(edmin.x, height - edmax.y, edmax.x, height - edmin.y));
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

            elementData = ResourceSystem::saveResourceProperties(ed);
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
                Sprite *ed = it.second;
                Vector4 bounds(ed->bounds());

                float yMin = height - bounds.w;
                float yMax = height - bounds.y;
                if(bounds.x < world.x && bounds.z > world.x &&
                    yMin < world.y && yMax > world.y) {
                    key = it.first;
                    break;
                }
            }

            m_controller->undoRedo()->push(new SelectSprite(key, m_controller));
        } else if(!isDrag) {
            m_controller->setDrag(true);
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(isDrag && !selected.isEmpty()) {
            cancelControl();
            m_controller->undoRedo()->push(new UpdateSprite(elementData, m_controller));
        }

        m_useBorder = false;
        m_controller->setDrag(false);

        if(selected.isEmpty() && m_currentPoint != m_startPoint) {
            uint32_t xmin = MIN(m_startPoint.x, m_currentPoint.x);
            uint32_t xmax = MAX(m_startPoint.x, m_currentPoint.x);
            uint32_t ymin = height - MAX(m_startPoint.y, m_currentPoint.y);
            uint32_t ymax = height - MIN(m_startPoint.y, m_currentPoint.y);

            m_controller->undoRedo()->push(new CreateSprite(Vector2(xmin, ymin), Vector2(xmax, ymax), m_controller));
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
    return ".embedded/composer.png/Transform";
}

TString SpriteTool::image() const {
    return ":/Images/editor/Transform.png";
}

TString SpriteTool::name() const {
    return "Resize";
}

TString SpriteTool::component() const {
    return SpriteRender::metaClass()->name();
}
