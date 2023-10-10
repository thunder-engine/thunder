#include "spritecontroller.h"

#include <QInputEvent>
#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>

#include <gizmos.h>
#include <input.h>

#define SCALE 100.0f

SpriteController::SpriteController(QWidget *view) :
        CameraController(),
        m_settings(nullptr),
        m_width(0),
        m_height(0),
        m_drag(false) {

    Camera *cam = camera();
    if(cam) {
        cam->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));
        cam->setOrthoSize(SCALE);
        cam->setFocal(SCALE);
    }
}

void SpriteController::setImportSettings(TextureImportSettings *settings) {
    m_settings = settings;
}

void SpriteController::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

void SpriteController::selectElements(const QStringList &list) {
    m_list = list;

    m_elementList.clear();
    if(m_settings) {
        for(auto &it : m_list) {
            m_elementList.push_back(m_settings->elements().value(it));
        }
    }

    if(m_list.isEmpty()) {
        emit selectionChanged(QString());
    } else {
        emit selectionChanged(m_list.front());
    }
}

QStringList &SpriteController::selectedElements() {
    return m_list;
}

void SpriteController::update() {
    if(Input::isKeyDown(Input::KEY_DELETE)) {
        if(!m_list.isEmpty()) {
            UndoManager::instance()->push(new DestroySprites(m_list, this));
        }
    }

    Vector4 pos(Input::mousePosition());
    if(m_settings && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        if(Handles::s_Axes == 0) {
            QString key;
            Vector2 world = mapToScene(Vector2(pos.z, pos.w));

            for(auto &it : m_settings->elements().keys()) {
                QRect r = m_settings->elements().value(it).m_rect;
                if(r.contains(world.x, world.y)) {
                    key = it;
                    break;
                }
            }

            if(key.isEmpty()) {
                selectElements({});
                m_startPoint = world;
                m_currentPoint = world;
            } else {
                UndoManager::instance()->push(new SelectSprites({key}, this));
            }
        }
    }

    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        if(m_drag && m_settings && !m_list.isEmpty()) {
            QList<TextureImportSettings::Element> temp;
            for(int i = 0; i < m_list.size(); i++) {
                temp.push_back(m_settings->elements().value(m_list.at(i)));
                m_settings->setElement(m_elementList.at(i), m_list.at(i));
            }
            UndoManager::instance()->push(new UpdateSprites(m_list, temp, this));
        }
        m_drag = false;
        if(m_currentPoint != m_startPoint) {
            TextureImportSettings::Element element;
            element.m_rect = makeRect(m_startPoint, m_currentPoint);
            UndoManager::instance()->push(new CreateSprite(element, this));
        }
        m_startPoint = m_currentPoint;
    }

    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(m_settings && Input::isMouseButton(Input::MOUSE_LEFT)) {
        m_drag = true;

        if(m_list.isEmpty()) {
            m_currentPoint = mapToScene(Handles::s_Mouse);
        } else {
            Vector2 p = mapToScene(Handles::s_Mouse);
            Vector2 delta = p - m_save;
            TextureImportSettings::Element element = m_settings->elements().value(m_list.front());
            QRect rect = element.m_rect;

            if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                rect.setTop(rect.top() + delta.y);
                rect.setBottom(rect.bottom() + delta.y);
                rect.setLeft(rect.left() + delta.x);
                rect.setRight(rect.right() + delta.x);
            } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
                QPoint v = rect.bottomRight();
                rect.setBottomRight(QPoint(v.x() + delta.x, v.y() + delta.y));
            } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
                QPoint v = rect.bottomLeft();
                rect.setBottomLeft(QPoint(v.x() + delta.x, v.y() + delta.y));
            } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
                QPoint v = rect.topRight();
                rect.setTopRight(QPoint(v.x() + delta.x, v.y() + delta.y));
            } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
                QPoint v = rect.topLeft();
                rect.setTopLeft(QPoint(v.x() + delta.x, v.y() + delta.y));
            } else if(Handles::s_Axes == Handles::POINT_T) {
                rect.setBottom(rect.bottom() + delta.y);
            } else if(Handles::s_Axes == Handles::POINT_B) {
                rect.setTop(rect.top() + delta.y);
            } else if(Handles::s_Axes == Handles::POINT_L) {
                rect.setLeft(rect.left() + delta.x);
            } else if(Handles::s_Axes == Handles::POINT_R) {
                rect.setRight(rect.right() + delta.x);
            }

            element.m_rect = makeRect(Vector2(rect.topLeft().x(), rect.topLeft().y()),
                                      Vector2(rect.bottomRight().x(), rect.bottomRight().y()));
            m_settings->setElement(element, m_list.front());
        }
    }
    m_save = mapToScene(Handles::s_Mouse);
}

void SpriteController::drawHandles() {
    if(m_settings) {
        Qt::CursorShape shape = Qt::ArrowCursor;

        Camera *cam = Camera::current();
        if(cam) {
            HandleTools::s_View = cam->viewMatrix();
            HandleTools::s_Projection = cam->projectionMatrix();
        }

        for(auto it : m_settings->elements().keys()) {
            QRectF r = mapRect(m_settings->elements().value(it).m_rect);
            if(m_list.indexOf(it) > -1) {
                QRect tmp = m_settings->elements().value(it).m_rect;
                tmp.setLeft(tmp.left()     + m_settings->elements().value(it).m_borderL);
                tmp.setRight(tmp.right()   - m_settings->elements().value(it).m_borderR);
                tmp.setTop(tmp.top()       + m_settings->elements().value(it).m_borderB);
                tmp.setBottom(tmp.bottom() - m_settings->elements().value(it).m_borderT);

                QRectF b = mapRect(tmp);

                int axis;
                Handles::s_Color = Handles::s_zColor;
                Handles::rectTool(Vector3(r.x(), r.y(), 0.0f), Vector3(r.width(), r.height(), 0), axis, m_drag);

                Gizmos::drawRectangle(Vector3(b.x(), b.y(), 0.0f), Vector2(b.width(), b.height()), Handles::s_yColor);

                Vector3 tr0 = Vector3(r.width() * 0.5f + r.x(), b.height() * 0.5f + b.y(), 0.0f);
                Vector3 tl0 = Vector3(r.width() *-0.5f + r.x(), b.height() * 0.5f + b.y(), 0.0f);
                Vector3 br0 = Vector3(r.width() * 0.5f + r.x(), b.height() *-0.5f + b.y(), 0.0f);
                Vector3 bl0 = Vector3(r.width() *-0.5f + r.x(), b.height() *-0.5f + b.y(), 0.0f);
                Vector3 tr1 = Vector3(b.width() * 0.5f + b.x(), r.height() * 0.5f + r.y(), 0.0f);
                Vector3 tl1 = Vector3(b.width() *-0.5f + b.x(), r.height() * 0.5f + r.y(), 0.0f);
                Vector3 br1 = Vector3(b.width() * 0.5f + b.x(), r.height() *-0.5f + r.y(), 0.0f);
                Vector3 bl1 = Vector3(b.width() *-0.5f + b.x(), r.height() *-0.5f + r.y(), 0.0f);

                Gizmos::drawLines({tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0, 1, 2, 3, 4, 5, 6, 7},
                                  Vector4(Handles::s_yColor.x, Handles::s_yColor.y, Handles::s_yColor.z, 0.5f));

                if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                    shape = Qt::SizeAllCursor;
                } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
                    shape = Qt::SizeBDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == Handles::POINT_T || Handles::s_Axes == Handles::POINT_B) {
                    shape = Qt::SizeVerCursor;
                } else if(Handles::s_Axes == Handles::POINT_L || Handles::s_Axes == Handles::POINT_R) {
                    shape = Qt::SizeHorCursor;
                }

            } else {
                Gizmos::drawRectangle(Vector3(r.x(), r.y(), 0.0f), Vector2(r.width(), r.height()), Handles::s_Grey);
            }
        }
        if(m_currentPoint != m_startPoint) {
            QRectF r = mapRect(makeRect(m_startPoint, m_currentPoint));
            Gizmos::drawRectangle(Vector3(r.x(), r.y(), 0.0f), Vector2(r.width(), r.height()), Handles::s_zColor);
        }
        Handles::s_Color = Handles::s_Normal;

        if(shape != Qt::ArrowCursor) {
            emit setCursor(QCursor(shape));
        } else if(!m_drag) {
            emit unsetCursor();
        }
    }
}

Vector2 SpriteController::mapToScene(const Vector2 &screen) {
    Vector3 world = Camera::unproject(Vector3(screen, 0.0f), m_activeCamera->viewMatrix(), m_activeCamera->projectionMatrix());
    world.x += SCALE * 0.5f;
    world.y += SCALE * 0.5f;

    world.x = world.x / SCALE * m_width;
    world.y = world.y / SCALE * m_height;

    return Vector2(world.x, world.y);
}

QRect SpriteController::makeRect(const Vector2 &p1, const Vector2 &p2) {
    QRect rect;
    if(p2.x < p1.x) {
        rect.setLeft(p2.x);
        rect.setRight(p1.x);

        if(Handles::s_Axes == Handles::POINT_R) {
            Handles::s_Axes = Handles::POINT_L;
        } else if(Handles::s_Axes == Handles::POINT_L) {
            Handles::s_Axes = Handles::POINT_R;
        }
    } else {
        rect.setLeft(p1.x);
        rect.setRight(p2.x);
    }

    if(p2.y < p1.y) {
        rect.setTop(p2.y - 1);
        rect.setBottom(p1.y - 1);

        if(Handles::s_Axes == Handles::POINT_T) {
            Handles::s_Axes = Handles::POINT_B;
        } else if(Handles::s_Axes == Handles::POINT_B) {
            Handles::s_Axes = Handles::POINT_T;
        } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
            Handles::s_Axes = Handles::POINT_B | Handles::POINT_L;
        } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
            Handles::s_Axes = Handles::POINT_T | Handles::POINT_R;
        } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
            Handles::s_Axes = Handles::POINT_B | Handles::POINT_R;
        } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
            Handles::s_Axes = Handles::POINT_T | Handles::POINT_L;
        }

    } else {
        rect.setTop(p1.y);
        rect.setBottom(p2.y);
    }
    return rect;
}

QRectF SpriteController::mapRect(const QRectF &rect) {
    QRectF result;
    float width = (rect.width() / m_width) * SCALE;
    float height = (rect.height() / m_height) * SCALE;
    result.setX((rect.x() / m_width - 0.5f) * SCALE + width * 0.5f);
    result.setY((rect.y() / m_height - 0.5f) * SCALE + height * 0.5f);
    result.setWidth(width);
    result.setHeight(height);
    return result;
}

SelectSprites::SelectSprites(const QStringList &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements) {
}
void SelectSprites::undo() {
    redo();
}
void SelectSprites::redo() {
    QStringList temp = m_controller->selectedElements();
    m_controller->selectElements(m_list);
    m_list = temp;
}

CreateSprite::CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group) :
    UndoSprite(ctrl, QObject::tr("Create Sprite Element"), group),
    m_rect(rect) {
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
        m_uuid = settings->setElement(m_rect, m_uuid);
        m_list = m_controller->selectedElements();
        m_controller->selectElements({m_uuid});
    }
}

DestroySprites::DestroySprites(const QStringList &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements) {
}
void DestroySprites::undo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        for(int32_t i = 0; i < m_rects.size(); i++) {
            settings->setElement(m_rects.at(i), m_list.at(i));
        }
        m_controller->selectElements(m_list);
    }
}
void DestroySprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        m_rects.clear();
        for(auto &it : m_list) {
            m_rects.push_back(settings->elements().value(it));
            settings->removeElement(it);
        }
        m_controller->selectElements({});
    }
}

UpdateSprites::UpdateSprites(const QStringList &elements, const QList<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_list(elements),
    m_rects(list) {
}
void UpdateSprites::undo() {
    redo();
}
void UpdateSprites::redo() {
    TextureImportSettings *settings = m_controller->settings();
    if(settings) {
        QList<TextureImportSettings::Element> temp;
        for(int32_t i = 0; i < m_rects.size(); i++) {
            temp.push_back(settings->elements().value(m_list.at(i)));
            settings->setElement(m_rects.at(i), m_list.at(i));
        }
        m_rects = temp;
    }
}
