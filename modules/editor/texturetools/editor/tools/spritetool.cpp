#include "spritetool.h"

#include <gizmos.h>

#include <actor.h>
#include <transform.h>
#include <camera.h>
#include <renderable.h>
#include <renderable.h>
#include <input.h>
#include <editor/viewport/handles.h>

#include "../spritecontroller.h"

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

void SpriteTool::beginControl() {
    m_savedPoint = m_currentPoint;

    for(auto &it : m_settings->elements()) {
        it.second.m_saveBorderMin = it.second.m_borderMin;
        it.second.m_saveBorderMax = it.second.m_borderMax;

        it.second.m_saveMin = it.second.m_min;
        it.second.m_saveMax = it.second.m_max;

        it.second.m_savePivot =it.second.m_pivot;
    }

    m_useBorder =
            (m_borderAxes == (Handles::POINT_L | Handles::POINT_T)) ||
            (m_borderAxes == (Handles::POINT_L | Handles::POINT_B)) ||
            (m_borderAxes == (Handles::POINT_R | Handles::POINT_T)) ||
            (m_borderAxes == (Handles::POINT_R | Handles::POINT_B)) ||
            m_borderAxes == Handles::POINT_L || m_borderAxes == Handles::POINT_R ||
            m_borderAxes == Handles::POINT_T || m_borderAxes == Handles::POINT_B;
}

void SpriteTool::cancelControl() {
    for(auto &it : m_settings->elements()) {
        it.second.m_borderMin = it.second.m_saveBorderMin;
        it.second.m_borderMax = it.second.m_saveBorderMax;

        it.second.m_min = it.second.m_saveMin;
        it.second.m_max = it.second.m_saveMax;

        it.second.m_pivot = it.second.m_savePivot;
    }
}

void SpriteTool::update(bool pivot, bool local, bool snap) {
    bool isDrag = m_controller->isDrag();
    Qt::CursorShape shape = Qt::ArrowCursor;

    m_currentPoint = m_controller->world();

    TextureImportSettings::Element element;

    std::string selected(m_controller->selectedElement());
    for(auto &it : m_settings->elements()) {
        AABBox rect;
        rect.setBox(Vector3(it.second.m_min, 0.0f), Vector3(it.second.m_max, 0.0f));

        if(m_controller->isSelected(it.first)) {
            Vector3 tl0(it.second.m_min.x, it.second.m_max.y - it.second.m_borderMax.y, 0.0f);
            Vector3 tr0(it.second.m_max.x, it.second.m_max.y - it.second.m_borderMax.y, 0.0f);
            Vector3 bl0(it.second.m_min.x, it.second.m_min.y + it.second.m_borderMin.y, 0.0f);
            Vector3 br0(it.second.m_max.x, it.second.m_min.y + it.second.m_borderMin.y, 0.0f);

            Vector3 tl1(it.second.m_min.x + it.second.m_borderMin.x, it.second.m_max.y, 0.0f);
            Vector3 tr1(it.second.m_max.x - it.second.m_borderMax.x, it.second.m_max.y, 0.0f);
            Vector3 bl1(it.second.m_min.x + it.second.m_borderMin.x, it.second.m_min.y, 0.0f);
            Vector3 br1(it.second.m_max.x - it.second.m_borderMax.x, it.second.m_min.y, 0.0f);

            Gizmos::drawLines({tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0, 1, 2, 3, 4, 5, 6, 7}, Handles::s_yColor);

            AABBox sub;
            sub.setBox(Vector3(it.second.m_min.x + it.second.m_borderMin.x, it.second.m_min.y + it.second.m_borderMin.y, 0.0f),
                       Vector3(it.second.m_max.x - it.second.m_borderMax.x, it.second.m_max.y - it.second.m_borderMax.y, 0.0f));

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

                Vector2 min(it.second.m_min);
                Vector2 max(it.second.m_max);

                if(m_useBorder) {
                    min = it.second.m_borderMin;
                    max = it.second.m_borderMax;
                }

                if(axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                    min += delta;
                    max += delta;
                } else if(axes == (Handles::POINT_T | Handles::POINT_R)) {
                    max += m_useBorder ? -delta : delta;
                } else if(axes == (Handles::POINT_T | Handles::POINT_L)) {
                    min.x += delta.x;
                    max.y += m_useBorder ? -delta.y : delta.y;
                } else if(axes == (Handles::POINT_B | Handles::POINT_R)) {
                    max.x += m_useBorder ? -delta.x : delta.x;
                    min.y += delta.y;
                } else if(axes == (Handles::POINT_B | Handles::POINT_L)) {
                    min += delta;
                } else if(axes == Handles::POINT_T) {
                    max.y += m_useBorder ? -delta.y : delta.y;
                } else if(axes == Handles::POINT_B) {
                    min.y += delta.y;
                } else if(axes == Handles::POINT_L) {
                    min.x += delta.x;
                } else if(axes == Handles::POINT_R) {
                    max.x += m_useBorder ? -delta.x : delta.x;
                }

                if(m_useBorder) {
                    Vector2 size(it.second.m_max - it.second.m_min);

                    it.second.m_borderMin.x = CLAMP(min.x, 0.0f, size.x);
                    it.second.m_borderMin.y = CLAMP(min.y, 0.0f, size.y - max.y);

                    it.second.m_borderMax.x = CLAMP(max.x, 0.0f, size.x - min.x);
                    it.second.m_borderMax.y = CLAMP(max.y, 0.0f, size.y);
                } else {
                    it.second.m_min.x = MAX(min.x, 0.0f);
                    it.second.m_min.y = MAX(min.y, 0.0f);
                    it.second.m_max.x = MIN(max.x, m_controller->width());
                    it.second.m_max.y = MIN(max.y, m_controller->height());
                }

                m_controller->updated();

                m_savedPoint = m_currentPoint;
            }

            if(axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                if(m_borderAxes != 0 && m_borderAxes != (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                    axes = m_borderAxes;
                }
                shape = Qt::SizeAllCursor;
            }

            if(axes == (Handles::POINT_T | Handles::POINT_R)) {
                shape = Qt::SizeBDiagCursor;
            } else if(axes == (Handles::POINT_T | Handles::POINT_L)) {
                shape = Qt::SizeFDiagCursor;
            } else if(axes == (Handles::POINT_B | Handles::POINT_R)) {
                shape = Qt::SizeFDiagCursor;
            } else if(axes == (Handles::POINT_B | Handles::POINT_L)) {
                shape = Qt::SizeBDiagCursor;
            } else if(axes == Handles::POINT_T || axes == Handles::POINT_B) {
                shape = Qt::SizeVerCursor;
            } else if(axes == Handles::POINT_L || axes == Handles::POINT_R) {
                shape = Qt::SizeHorCursor;
            }

            element = it.second;
        } else {
            Gizmos::drawRectangle(rect.center, Vector2(rect.extent.x * 2.0f, rect.extent.y * 2.0f), Handles::s_xColor);
        }
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        m_currentPoint = m_startPoint = m_controller->world();

        if(Handles::s_Axes == 0) {
            std::string key;

            Vector3 world = m_controller->world();
            for(auto &it : m_settings->elements()) {
                if(it.second.m_min.x < world.x && it.second.m_max.x > world.x &&
                   it.second.m_min.y < world.y && it.second.m_max.y > world.y) {
                    key = it.first;
                    break;
                }
            }

            if(key.empty()) {
                m_controller->itemsSelected({});
                UndoManager::instance()->push(new SelectSprite("", m_controller));
            } else {
                m_item.setKey(key);
                m_controller->itemsSelected({&m_item});
                UndoManager::instance()->push(new SelectSprite(key, m_controller));
            }
        } else if(!isDrag) {
            m_controller->setDrag(true);
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(isDrag && !selected.empty()) {
            cancelControl();

            UndoManager::instance()->push(new UpdateSprite(element, m_controller));
        }

        m_useBorder = false;
        m_controller->setDrag(false);

        if(selected.empty() && m_currentPoint != m_startPoint) {
            TextureImportSettings::Element element;
            element.m_min = Vector2(MIN(m_startPoint.x, m_currentPoint.x), MIN(m_startPoint.y, m_currentPoint.y));
            element.m_max = Vector2(MAX(m_startPoint.x, m_currentPoint.x), MAX(m_startPoint.y, m_currentPoint.y));

            UndoManager::instance()->push(new CreateSprite(element, m_controller));

            m_startPoint = m_currentPoint;
        }
    }

    // Creation of sprite
    if(selected.empty() && Input::isMouseButton(Input::MOUSE_LEFT) && m_currentPoint != m_startPoint) {
        AABBox bb;
        bb.setBox(m_startPoint, m_currentPoint);

        Gizmos::drawRectangle(bb.center, Vector2(bb.extent.x * 2.0f, bb.extent.y * 2.0f), Handles::s_yColor);
    }

    if(isDrag && Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        cancelControl();
        m_useBorder = false;
        m_controller->setDrag(false);
    }

    if(!selected.empty() && Input::isKeyDown(Input::KEY_DELETE)) {
        UndoManager::instance()->push(new DestroySprite(m_controller));
    }

    m_cursor = shape;
}

QString SpriteTool::icon() const {
    return ":/Images/editor/Transform.png";
}

QString SpriteTool::name() const {
    return "Resize";
}
