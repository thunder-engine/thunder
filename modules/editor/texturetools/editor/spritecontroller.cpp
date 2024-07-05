#include "spritecontroller.h"

#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>

#include <gizmos.h>
#include <input.h>

SpriteController::SpriteController(QWidget *view) :
        CameraController(),
        m_settings(nullptr),
        m_width(0),
        m_height(0),
        m_drag(false),
        m_borderAxes(0) {

}

void SpriteController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    Camera *cam = camera();
    if(cam) {
        cam->transform()->setPosition(Vector3(m_width * 0.5f, m_height * 0.5f, 1.0f));
        cam->setOrthoSize(m_height);
        cam->setFocal(m_height);
    }
}

void SpriteController::selectElements(const std::list<std::string> &list) {
    m_list = list;

    m_elementList.clear();
    if(m_settings) {
        auto elements = m_settings->elements();
        for(auto &it : m_list) {
            auto result = elements.find(it);
            if(result != elements.end()) {
                m_elementList.push_back(result->second);
            }
        }
    }

    if(m_list.empty()) {
        emit selectionChanged(QString());
    } else {
        emit selectionChanged(m_list.front().c_str());
    }
}

const std::list<std::string> &SpriteController::selectedElements() {
    return m_list;
}

void SpriteController::update() {
    Vector4 pos(Input::mousePosition());
    Vector3 world = m_activeCamera->unproject(Vector3(pos.z, pos.w, 0.0f));
    world.x = CLAMP(world.x, 0.0f, m_width);
    world.y = CLAMP(world.y, 0.0f, m_height);

    Handles::s_Mouse = Vector2(pos.x, pos.y);
    Handles::s_Screen = m_screenSize;

    CameraController::update();

    if(Input::isKeyDown(Input::KEY_DELETE)) {
        if(!m_list.empty()) {
            UndoManager::instance()->push(new DestroySprites(m_list, this));
        }
    }

    if(m_settings && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        m_startPoint = m_currentPoint = world;

        if(Handles::s_Axes == 0) {
            std::string key;
            for(auto &it : m_settings->elements()) {
                if(it.second.m_min.x < world.x && it.second.m_max.x > world.x &&
                   it.second.m_min.y < world.y && it.second.m_max.y > world.y) {
                    key = it.first;
                    break;
                }
            }

            if(key.empty()) {
                selectElements({});
            } else {
                UndoManager::instance()->push(new SelectSprites({key}, this));
            }
        }

        if(!m_elementList.empty()) {
            TextureImportSettings::Element element = m_elementList.front();

            m_min = element.m_min;
            m_max = element.m_max;

            m_borderMin = element.m_borderMin;
            m_borderMax = element.m_borderMax;
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(m_drag && m_settings && !m_list.empty()) {
            UndoManager::instance()->push(new UpdateSprites(m_list, m_elementList, this));
        }
        m_drag = false;

        if(m_elementList.empty() && m_currentPoint != m_startPoint) {
            TextureImportSettings::Element element;
            element.m_min = Vector2(MIN(m_startPoint.x, m_currentPoint.x), MIN(m_startPoint.y, m_currentPoint.y));
            element.m_max = Vector2(MAX(m_startPoint.x, m_currentPoint.x), MAX(m_startPoint.y, m_currentPoint.y));

            UndoManager::instance()->push(new CreateSprite(element, this));
        }
        m_startPoint = m_currentPoint;
    }

    if(m_settings && Input::isMouseButton(Input::MOUSE_LEFT)) {
        m_currentPoint = world;

        if(!m_elementList.empty()) {
            Vector2 delta(int(m_currentPoint.x - m_startPoint.x), int(m_currentPoint.y - m_startPoint.y));

            if(delta.length() > 1.0f && !m_drag) {
                m_drag = true;
            }

            if(m_drag) {
                Vector2 min(m_min);
                Vector2 max(m_max);

                bool useBorder =
                        (m_borderAxes == (Handles::POINT_L | Handles::POINT_T) && m_borderAxes != Handles::s_Axes) ||
                        (m_borderAxes == (Handles::POINT_L | Handles::POINT_B) && m_borderAxes != Handles::s_Axes) ||
                        (m_borderAxes == (Handles::POINT_R | Handles::POINT_T) && m_borderAxes != Handles::s_Axes) ||
                        (m_borderAxes == (Handles::POINT_R | Handles::POINT_B) && m_borderAxes != Handles::s_Axes) ||
                        m_borderAxes == Handles::POINT_L || m_borderAxes == Handles::POINT_R ||
                        m_borderAxes == Handles::POINT_T || m_borderAxes == Handles::POINT_B;

                uint8_t axes = Handles::s_Axes;
                if(useBorder) {
                    axes = m_borderAxes;
                    min = m_borderMin;
                    max = m_borderMax;
                }

                if(axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                    min += delta;
                    max += delta;
                } else if(axes == (Handles::POINT_T | Handles::POINT_R)) {
                    max += useBorder ? -delta : delta;
                } else if(axes == (Handles::POINT_T | Handles::POINT_L)) {
                    min.x += delta.x;
                    max.y += useBorder ? -delta.y : delta.y;
                } else if(axes == (Handles::POINT_B | Handles::POINT_R)) {
                    max.x += useBorder ? -delta.x : delta.x;
                    min.y += delta.y;
                } else if(axes == (Handles::POINT_B | Handles::POINT_L)) {
                    min += delta;
                } else if(axes == Handles::POINT_T) {
                    max.y += useBorder ? -delta.y : delta.y;
                } else if(axes == Handles::POINT_B) {
                    min.y += delta.y;
                } else if(axes == Handles::POINT_L) {
                    min.x += delta.x;
                } else if(axes == Handles::POINT_R) {
                    max.x += useBorder ? -delta.x : delta.x;
                }

                TextureImportSettings::Element &element = m_elementList.front();

                if(useBorder) {
                    Vector2 size(element.m_max - element.m_min);

                    element.m_borderMin.x = CLAMP(min.x, 0.0f, size.x/* - max.x*/);
                    element.m_borderMin.y = CLAMP(min.y, 0.0f, size.y - max.y);

                    element.m_borderMax.x = CLAMP(max.x, 0.0f, size.x - min.x);
                    element.m_borderMax.y = CLAMP(max.y, 0.0f, size.y/* - min.y*/);
                } else {
                    element.m_min = min;
                    element.m_max = max;
                }
            }
        }
    }

    if(m_drag && m_settings && Input::isMouseButtonDown(Input::MOUSE_RIGHT)) {
        if(!m_elementList.empty()) {
            TextureImportSettings::Element &element = m_elementList.front();

            element.m_min = m_min;
            element.m_max = m_max;

            m_currentPoint = m_startPoint;
            m_drag = false;
        }
    }
}

void SpriteController::drawHandles() {
    if(m_settings) {
        Qt::CursorShape shape = Qt::ArrowCursor;

        for(auto &it : m_settings->elements()) {
            if(find(m_list.begin(), m_list.end(), it.first) == m_list.end()) {
                AABBox rect;
                rect.setBox(Vector3(it.second.m_min, 0.0f), Vector3(it.second.m_max, 0.0f));

                Gizmos::drawRectangle(rect.center, Vector2(rect.extent.x * 2.0f, rect.extent.y * 2.0f), Handles::s_Normal);
            }
        }

        for(auto &it : m_elementList) {
            AABBox rect;
            rect.setBox(Vector3(it.m_min, 0.0f), Vector3(it.m_max, 0.0f));

            AABBox bb;
            bb.setBox(Vector3(it.m_min.x + it.m_borderMin.x, it.m_min.y + it.m_borderMin.y, 0.0f),
                      Vector3(it.m_max.x - it.m_borderMax.x, it.m_max.y - it.m_borderMax.y, 0.0f));

            int axis;

            Vector3 tl0(it.m_min.x, it.m_max.y - it.m_borderMax.y, 0.0f);
            Vector3 tr0(it.m_max.x, it.m_max.y - it.m_borderMax.y, 0.0f);
            Vector3 bl0(it.m_min.x, it.m_min.y + it.m_borderMin.y, 0.0f);
            Vector3 br0(it.m_max.x, it.m_min.y + it.m_borderMin.y, 0.0f);

            Vector3 tl1(it.m_min.x + it.m_borderMin.x, it.m_max.y, 0.0f);
            Vector3 tr1(it.m_max.x - it.m_borderMax.x, it.m_max.y, 0.0f);
            Vector3 bl1(it.m_min.x + it.m_borderMin.x, it.m_min.y, 0.0f);
            Vector3 br1(it.m_max.x - it.m_borderMax.x, it.m_min.y, 0.0f);

            Gizmos::drawLines({tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0, 1, 2, 3, 4, 5, 6, 7}, Handles::s_yColor);

            Handles::s_Color = Handles::s_yColor;
            Handles::rectTool(bb.center, bb.extent * 2.0f, axis, true, m_drag);

            if(!m_drag) {
                m_borderAxes = Handles::s_Axes;
            }

            Handles::s_Color = Handles::s_zColor;
            Handles::rectTool(rect.center, rect.extent * 2.0f, axis, false, m_drag);

            bool useBorder =
                    m_borderAxes == Handles::POINT_L || m_borderAxes == Handles::POINT_R ||
                    m_borderAxes == Handles::POINT_T || m_borderAxes == Handles::POINT_B ||
                    m_borderAxes == (Handles::POINT_T | Handles::POINT_R) ||
                    m_borderAxes == (Handles::POINT_T | Handles::POINT_L) ||
                    m_borderAxes == (Handles::POINT_B | Handles::POINT_R) ||
                    m_borderAxes == (Handles::POINT_B | Handles::POINT_L);

            uint8_t axes = Handles::s_Axes;
            if(useBorder) {
                axes = m_borderAxes;
            }

            if(axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                shape = Qt::SizeAllCursor;
            } else if(axes == (Handles::POINT_T | Handles::POINT_R)) {
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
        }

        if(m_list.empty() && m_currentPoint != m_startPoint) {
            AABBox bb;
            bb.setBox(m_startPoint, m_currentPoint);
            Gizmos::drawRectangle(bb.center, Vector2(bb.extent.x * 2.0f, bb.extent.y * 2.0f), Handles::s_yColor);
        }
        Handles::s_Color = Handles::s_Normal;

        if(shape != Qt::ArrowCursor) {
            emit setCursor(QCursor(shape));
        } else if(!m_drag) {
            emit unsetCursor();
        }
    }
}

SelectSprites::SelectSprites(const std::list<std::string> &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements) {
}
void SelectSprites::undo() {
    redo();
}
void SelectSprites::redo() {
    std::list<std::string> temp = m_controller->selectedElements();
    m_controller->selectElements(m_list);
    m_list = temp;
}

CreateSprite::CreateSprite(const TextureImportSettings::Element &element, SpriteController *ctrl, QUndoCommand *group) :
    UndoSprite(ctrl, QObject::tr("Create Sprite Element"), group),
    m_element(element) {
}
void CreateSprite::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        settings->removeElement(m_uuid);
        m_controller->selectElements(m_list);
    }
}
void CreateSprite::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_uuid = settings->setElement(m_element, m_uuid);
        m_list = m_controller->selectedElements();
        m_controller->selectElements({m_uuid});
    }
}

DestroySprites::DestroySprites(const std::list<std::string> &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements) {
}
void DestroySprites::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        for(int32_t i = 0; i < m_elements.size(); i++) {
            settings->setElement(*next(m_elements.begin(), i), *next(m_list.begin(), i));
        }
        m_controller->selectElements(m_list);
    }
}
void DestroySprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_elements.clear();
        for(auto &it : m_list) {
            auto element = settings->elements().find(it);
            if(element != settings->elements().end()) {
                m_elements.push_back(element->second);
            }
            settings->removeElement(it);
        }
        m_controller->selectElements({});
    }
}

UpdateSprites::UpdateSprites(const std::list<std::string> &elements, const std::list<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements),
    m_elements(list) {
}
void UpdateSprites::undo() {
    redo();
}
void UpdateSprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        std::list<TextureImportSettings::Element> temp;
        for(int32_t i = 0; i < m_elements.size(); i++) {
            auto key = next(m_list.begin(), i);
            auto element = settings->elements().find(*key);
            if(element != settings->elements().end()) {
                TextureImportSettings::Element back = element->second;
                temp.push_back(back);
            }
            settings->setElement(*next(m_elements.begin(), i), *next(m_list.begin(), i));
        }
        m_elements = temp;
        m_controller->selectElements(m_list);
    }
}
