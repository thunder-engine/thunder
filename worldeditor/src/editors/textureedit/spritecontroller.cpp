#include "spritecontroller.h"

#include <QInputEvent>
#include <QCursor>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>
#include <editor/viewport/editorpipeline.h>

SpriteController::SpriteController(QWidget *view) :
        CameraCtrl(view),
        m_pPipeline(nullptr),
        m_pSettings(nullptr),
        m_Width(0),
        m_Height(0),
        m_Drag(false) {

    Camera *cam = camera();
    if(cam) {
        cam->actor()->transform()->setPosition(Vector3(0.0f, 0.0f, 1.0f));
        cam->setOrthoSize(SCALE);
        cam->setFocal(SCALE);
    }
}

SpriteController::~SpriteController() {
    delete m_pPipeline;
}

void SpriteController::init() {
    if(m_pPipeline) {
        delete m_pPipeline;
    }
    m_pPipeline = new EditorPipeline;
    m_pPipeline->setController(this);
    m_activeCamera->setPipeline(m_pPipeline);
}

void SpriteController::setImportSettings(TextureImportSettings *settings) {
    m_pSettings = settings;
}

void SpriteController::setSize(uint32_t width, uint32_t height) {
    m_Width = width;
    m_Height = height;
}

void SpriteController::selectElements(const QStringList &list) {
    m_List = list;

    m_ElementList.clear();
    if(m_pSettings) {
        for(auto &it : m_List) {
            m_ElementList.push_back(m_pSettings->elements().value(it));
        }
    }

    if(m_List.isEmpty()) {
        emit selectionChanged(QString());
    } else {
        emit selectionChanged(m_List.front());
    }
}

QStringList &SpriteController::selectedElements() {
    return m_List;
}

void SpriteController::drawHandles() {
    if(m_pSettings) {
        Qt::CursorShape shape = Qt::ArrowCursor;

        Handles::cleanDepth();
        for(auto it : m_pSettings->elements().keys()) {
            QRectF r = mapRect(m_pSettings->elements().value(it).m_Rect);
            if(m_List.indexOf(it) > -1) {
                QRect tmp = m_pSettings->elements().value(it).m_Rect;
                tmp.setLeft(tmp.left()     + m_pSettings->elements().value(it).m_BorderL);
                tmp.setRight(tmp.right()   - m_pSettings->elements().value(it).m_BorderR);
                tmp.setTop(tmp.top()       + m_pSettings->elements().value(it).m_BorderB);
                tmp.setBottom(tmp.bottom() - m_pSettings->elements().value(it).m_BorderT);

                QRectF b = mapRect(tmp);

                int axis;
                Handles::s_Color = Handles::s_zColor;
                Handles::rectTool(Vector3(r.x(), r.y(), 0.0f), Vector3(r.width(), r.height(), 0), axis, m_Drag);

                Handles::s_Color = Handles::s_yColor;
                Handles::drawRectangle(Vector3(b.x(), b.y(), 0.0f), Quaternion(), b.width(), b.height());

                Vector3 tr0 = Vector3(r.width() * 0.5f + r.x(), b.height() * 0.5f + b.y(), 0.0f);
                Vector3 tl0 = Vector3(r.width() *-0.5f + r.x(), b.height() * 0.5f + b.y(), 0.0f);
                Vector3 br0 = Vector3(r.width() * 0.5f + r.x(), b.height() *-0.5f + b.y(), 0.0f);
                Vector3 bl0 = Vector3(r.width() *-0.5f + r.x(), b.height() *-0.5f + b.y(), 0.0f);
                Vector3 tr1 = Vector3(b.width() * 0.5f + b.x(), r.height() * 0.5f + r.y(), 0.0f);
                Vector3 tl1 = Vector3(b.width() *-0.5f + b.x(), r.height() * 0.5f + r.y(), 0.0f);
                Vector3 br1 = Vector3(b.width() * 0.5f + b.x(), r.height() *-0.5f + r.y(), 0.0f);
                Vector3 bl1 = Vector3(b.width() *-0.5f + b.x(), r.height() *-0.5f + r.y(), 0.0f);

                Handles::s_Color = Vector4(Handles::s_yColor.x, Handles::s_yColor.y, Handles::s_yColor.z, 0.5f);
                Handles::drawLines(Matrix4(), {tr0, tl0, br0, bl0, tr1, br1, tl1, bl1}, {0, 1, 2, 3, 4, 5, 6, 7});

                if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                    shape = Qt::SizeAllCursor;
                } if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
                    shape = Qt::SizeBDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
                    shape = Qt::SizeFDiagCursor;
                } else if(Handles::s_Axes == Handles::POINT_T | Handles::s_Axes == Handles::POINT_B) {
                    shape = Qt::SizeVerCursor;
                } else if(Handles::s_Axes == Handles::POINT_L | Handles::s_Axes == Handles::POINT_R) {
                    shape = Qt::SizeHorCursor;
                }
            } else {
                Handles::s_Color = Handles::s_Grey;
                Handles::drawRectangle(Vector3(r.x(), r.y(), 0.0f), Quaternion(), r.width(), r.height());
            }
        }
        if(m_CurrentPoint != m_StartPoint) {
            Handles::s_Color = Handles::s_zColor;

            QRectF r = mapRect(makeRect(m_StartPoint, m_CurrentPoint));
            Handles::drawRectangle(Vector3(r.x(), r.y(), 0.0f), Quaternion(), r.width(), r.height());
        }
        Handles::s_Color = Handles::s_Normal;

        if(shape != Qt::ArrowCursor) {
            emit setCursor(QCursor(shape));
        } else if(!m_Drag) {
            emit unsetCursor();
        }
    }
}

void SpriteController::onInputEvent(QInputEvent *pe) {
    CameraCtrl::onInputEvent(pe);
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_Delete: {
                    if(!m_List.isEmpty()) {
                        UndoManager::instance()->push(new DestroySprites(m_List, this));
                    }
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(m_pSettings && e->buttons() == Qt::LeftButton) {
                if(Handles::s_Axes == 0) {
                    QString key;
                    QPoint world = mapToScene(e->pos());
                    for(auto &it : m_pSettings->elements().keys()) {
                        QRect r = m_pSettings->elements().value(it).m_Rect;
                        if(r.contains(world)) {
                            key = it;
                            break;
                        }
                    }

                    if(key.isEmpty()) {
                        selectElements({});
                        m_StartPoint = world;
                        m_CurrentPoint = world;
                    } else {
                        UndoManager::instance()->push(new SelectSprites({key}, this));
                    }
                }
            }
        } break;
        case QEvent::MouseButtonRelease: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->button() == Qt::LeftButton) {
                if(m_Drag && m_pSettings && !m_List.isEmpty()) {
                    QList<TextureImportSettings::Element> temp;
                    for(int i = 0; i < m_List.size(); i++) {
                        temp.push_back(m_pSettings->elements().value(m_List.at(i)));
                        m_pSettings->setElement(m_ElementList.at(i), m_List.at(i));
                    }
                    UndoManager::instance()->push(new UpdateSprites(m_List, temp, this));
                }
                m_Drag = false;
                if(m_CurrentPoint.x() != m_StartPoint.x() && m_CurrentPoint.y() != m_StartPoint.y()) {
                    TextureImportSettings::Element element;
                    element.m_Rect = makeRect(m_StartPoint, m_CurrentPoint);
                    UndoManager::instance()->push(new CreateSprite(element, this));
                }
                m_StartPoint = m_CurrentPoint;
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);

            Vector3 screen = Vector3(e->pos().x() / m_screenSize.x, 1.0f - e->pos().y() / m_screenSize.y, 0.0f);
            Handles::s_Mouse = Vector2(screen.x, screen.y);
            Handles::s_Screen = m_screenSize;

            if(m_pSettings && e->buttons() & Qt::LeftButton) {
                m_Drag = true;

                if(m_List.isEmpty()) {
                    m_CurrentPoint = mapToScene(e->pos());
                } else {
                    QPoint p = mapToScene(e->pos());
                    int32_t dx = p.x() - m_Save.x();
                    int32_t dy = p.y() - m_Save.y();
                    TextureImportSettings::Element element = m_pSettings->elements().value(m_List.front());
                    QRect rect = element.m_Rect;

                    if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
                        rect.setTop(rect.top() + dy);
                        rect.setBottom(rect.bottom() + dy);
                        rect.setLeft(rect.left() + dx);
                        rect.setRight(rect.right() + dx);
                    } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
                        QPoint v = rect.bottomRight();
                        rect.setBottomRight(QPoint(v.x() + dx, v.y() + dy));
                    } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
                        QPoint v = rect.bottomLeft();
                        rect.setBottomLeft(QPoint(v.x() + dx, v.y() + dy));
                    } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
                        QPoint v = rect.topRight();
                        rect.setTopRight(QPoint(v.x() + dx, v.y() + dy));
                    } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
                        QPoint v = rect.topLeft();
                        rect.setTopLeft(QPoint(v.x() + dx, v.y() + dy));
                    } else if(Handles::s_Axes == Handles::POINT_T) {
                        int v = rect.bottom();
                        rect.setBottom(v + dy);
                    } else if(Handles::s_Axes == Handles::POINT_B) {
                        int v = rect.top();
                        rect.setTop(v + dy);
                    } else if(Handles::s_Axes == Handles::POINT_L) {
                        int v = rect.left();
                        rect.setLeft(v + dx);
                    } else if(Handles::s_Axes == Handles::POINT_R) {
                        int v = rect.right();
                        rect.setRight(v + dx);
                    }

                    element.m_Rect = makeRect(rect.topLeft(), rect.bottomRight());
                    m_pSettings->setElement(element, m_List.front());
                }
            }
            m_Save = mapToScene(e->pos());
        } break;
        default: break;
    }
}

QPoint SpriteController::mapToScene(const QPoint &pos) {
    Vector3 screen((float)pos.x() / m_screenSize.x, 1.0f - (float)pos.y() / m_screenSize.y, 0.0f);

    Vector3 world = Camera::unproject(screen, m_activeCamera->viewMatrix(), m_activeCamera->projectionMatrix());
    world.x += SCALE * 0.5f;
    world.y += SCALE * 0.5f;

    world.x = world.x / SCALE * m_Width;
    world.y = world.y / SCALE * m_Height;

    return QPoint(world.x, world.y);
}

QRect SpriteController::makeRect(const QPoint &p1, const QPoint &p2) {
    QRect rect;
    if(p2.x() < p1.x()) {
        rect.setLeft(p2.x());
        rect.setRight(p1.x());

        if(Handles::s_Axes == Handles::POINT_R) {
            Handles::s_Axes = Handles::POINT_L;
        } else if(Handles::s_Axes == Handles::POINT_L) {
            Handles::s_Axes = Handles::POINT_R;
        }
    } else {
        rect.setLeft(p1.x());
        rect.setRight(p2.x());
    }

    if(p2.y() < p1.y()) {
        rect.setTop(p2.y() - 1);
        rect.setBottom(p1.y() - 1);

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
        rect.setTop(p1.y());
        rect.setBottom(p2.y());
    }
    return rect;
}

QRectF SpriteController::mapRect(const QRectF &rect) {
    QRectF result;
    float width = (rect.width() / m_Width) * SCALE;
    float height = (rect.height() / m_Height) * SCALE;
    result.setX((rect.x() / m_Width - 0.5f) * SCALE + width * 0.5f);
    result.setY((rect.y() / m_Height - 0.5f) * SCALE + height * 0.5f);
    result.setWidth(width);
    result.setHeight(height);
    return result;
}

SelectSprites::SelectSprites(const QStringList &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_List(elements) {
}
void SelectSprites::undo() {
    redo();
}
void SelectSprites::redo() {
    QStringList temp = m_pController->selectedElements();
    m_pController->selectElements(m_List);
    m_List = temp;
}

CreateSprite::CreateSprite(const TextureImportSettings::Element &rect, SpriteController *ctrl, QUndoCommand *group) :
    UndoSprite(ctrl, QObject::tr("Create Sprite Element"), group),
    m_Rect(rect) {
}
void CreateSprite::undo() {
    TextureImportSettings *settings = m_pController->settings();
    if(settings) {
        settings->removeElement(m_Uuid);
        m_pController->selectElements(m_List);
    }
}
void CreateSprite::redo() {
    TextureImportSettings *settings = m_pController->settings();
    if(settings) {
        m_Uuid = settings->setElement(m_Rect, m_Uuid);
        m_List = m_pController->selectedElements();
        m_pController->selectElements({m_Uuid});
    }
}

DestroySprites::DestroySprites(const QStringList &elements, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_List(elements) {
}
void DestroySprites::undo() {
    TextureImportSettings *settings = m_pController->settings();
    if(settings) {
        for(int32_t i = 0; i < m_Rects.size(); i++) {
            settings->setElement(m_Rects.at(i), m_List.at(i));
        }
        m_pController->selectElements(m_List);
    }
}
void DestroySprites::redo() {
    TextureImportSettings *settings = m_pController->settings();
    if(settings) {
        m_Rects.clear();
        for(auto &it : m_List) {
            m_Rects.push_back(settings->elements().value(it));
            settings->removeElement(it);
        }
        m_pController->selectElements({});
    }
}

UpdateSprites::UpdateSprites(const QStringList &elements, const QList<TextureImportSettings::Element> &list, SpriteController *ctrl, const QString &name, QUndoCommand *group) :
    UndoSprite(ctrl, name, group),
    m_List(elements),
    m_Rects(list) {
}
void UpdateSprites::undo() {
    redo();
}
void UpdateSprites::redo() {
    TextureImportSettings *settings = m_pController->settings();
    if(settings) {
        QList<TextureImportSettings::Element> temp;
        for(int32_t i = 0; i < m_Rects.size(); i++) {
            temp.push_back(settings->elements().value(m_List.at(i)));
            settings->setElement(m_Rects.at(i), m_List.at(i));
        }
        m_Rects = temp;
    }
}
