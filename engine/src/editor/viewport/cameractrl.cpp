#include "editor/viewport/cameractrl.h"
#include "editor/viewport/handles.h"

#include <QMenu>

#include <engine.h>
#include <timer.h>
#include <float.h>

#include <components/scenegraph.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/spriterender.h>
#include <components/textrender.h>

#define DT 0.0625f

CameraCtrl::CameraCtrl() :
        m_cameraMove(MoveTypes::MOVE_IDLE),
        m_viewSide(ViewSide::VIEW_SCENE),
        m_blockMove(false),
        m_blockRotation(false),
        m_cameraFree(true),
        m_cameraFreeSaved(true),
        m_rotationTransfer(false),
        m_orthoWidthTarget(-1.0f),
        m_focalLengthTarget(-1.0f),
        m_transferProgress(1.0f),
        m_camera(nullptr),
        m_activeCamera(nullptr),
        m_activeRootObject(nullptr) {

    m_camera = Engine::composeActor("Camera", "Camera", nullptr);
    m_activeCamera = static_cast<Camera *>(m_camera->component("Camera"));
    m_activeCamera->setFocal(10.0f);
    m_activeCamera->setOrthoSize(10.0f);

    m_activeCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0));

    m_camera->transform()->setPosition(Vector3(0.0, 0.0, 20.0));
}

void CameraCtrl::drawHandles() {
    drawHelpers(*m_activeRootObject);
}

void CameraCtrl::resize(int32_t width, int32_t height) {
    m_screenSize = Vector2(width, height);
}

Object::ObjectList CameraCtrl::selected() {
    return Object::ObjectList();
}

void CameraCtrl::select(Object &object) {

}

void CameraCtrl::update() {
    if(!m_blockMove) {
        if(m_cameraMove & MOVE_FORWARD) {
            m_cameraSpeed.z += 0.1f;
        }

        if(m_cameraMove & MOVE_BACKWARD) {
            m_cameraSpeed.z -= 0.1f;
        }

        if(m_cameraMove & MOVE_LEFT) {
            m_cameraSpeed.x -= 0.1f;
        }

        if(m_cameraMove & MOVE_RIGHT) {
            m_cameraSpeed.x += 0.1f;
        }
    }

    if(m_activeCamera) {
        Transform *t = m_camera->transform();
        if(m_transferProgress < 1.0f) {
            if(m_rotationTransfer) {
                cameraRotate(t->rotation() - MIX(t->rotation(), m_rotation, m_transferProgress));
            } else {
                t->setPosition(MIX(t->position(), m_positionTarget, m_transferProgress));
            }

            if(m_orthoWidthTarget > 0.0f) {
                m_activeCamera->setOrthoSize(MIX(m_activeCamera->orthoSize(), m_orthoWidthTarget, m_transferProgress));
            }

            if(m_focalLengthTarget > 0.0f) {
                m_activeCamera->setFocal(MIX(m_activeCamera->focal(), m_focalLengthTarget, m_transferProgress));
            }

            m_transferProgress += 2.0f * DT;

            if(m_transferProgress >= 1.0f) {
                m_cameraFree = m_cameraFreeSaved;

                if(m_rotationTransfer) {
                    cameraRotate(t->rotation() - m_rotation);
                } else {
                    t->setPosition(m_positionTarget);
                }

                if(m_orthoWidthTarget > 0.0f) {
                    m_activeCamera->setOrthoSize(m_orthoWidthTarget);
                }

                if(m_focalLengthTarget > 0.0f) {
                    m_activeCamera->setFocal(m_focalLengthTarget);
                }
            }
        }

        if(m_cameraSpeed.x != 0.0f || m_cameraSpeed.y != 0.0f || m_cameraSpeed.z != 0.0f) {
            Vector3 pos = t->position();
            Vector3 dir = t->quaternion() * Vector3(0.0, 0.0, 1.0);
            dir.normalize();

            Vector3 delta = (dir * m_cameraSpeed.z) + dir.cross(Vector3(0.0f, 1.0f, 0.0f)) * m_cameraSpeed.x;
            t->setPosition(pos - delta * m_activeCamera->focal() * 0.1f);

            m_cameraSpeed -= m_cameraSpeed * 10.0f * DT;
            if(m_cameraSpeed.length() <= .01f) {
                m_cameraSpeed = Vector3();
            }
        }
    }
}

void CameraCtrl::onOrthographic(bool flag) {
    if(m_activeCamera->orthographic() != flag) {
        m_activeCamera->setOrthographic(flag);
        if(flag) {
            m_rotation = m_camera->transform()->rotation();
            frontSide();
        } else {
            doRotation(m_rotation);
        }
        m_orthoWidthTarget = -1.0f;
        m_focalLengthTarget = -1.0f;
    }
}

void CameraCtrl::setFocusOn(Actor *actor, float &bottom) {
    bottom  = 0;
    if(actor) {
        AABBox bb(Vector3(FLT_MAX), Vector3(-FLT_MAX));
        for(auto it : actor->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }
        float radius = (bb.radius * 2.0f) / sinf(m_activeCamera->fov() * DEG2RAD);

        Vector3 min, max;
        bb.box(min, max);
        bottom = min.y;

        m_focalLengthTarget = radius;
        m_orthoWidthTarget = radius;
        Transform *camera = m_camera->transform();
        m_positionTarget = bb.center + camera->quaternion() * Vector3(0.0, 0.0, radius);
        m_transferProgress = 0.0f;
        m_rotationTransfer = false;
    }
}

void CameraCtrl::createMenu(QMenu *menu) {
    if(menu) {
        menu->addAction(tr("Front View"), this, SLOT(frontSide()));
        menu->addAction(tr("Back View"), this, SLOT(backSide()));
        menu->addAction(tr("Left View"), this, SLOT(leftSide()));
        menu->addAction(tr("Right View"), this, SLOT(rightSide()));
        menu->addAction(tr("Top View"), this, SLOT(topSide()));
        menu->addAction(tr("Bottom View"), this, SLOT(bottomSide()));
    }
}

void CameraCtrl::frontSide() {
    doRotation(Vector3());
    m_viewSide = ViewSide::VIEW_FRONT;
}

void CameraCtrl::backSide() {
    doRotation(Vector3( 0.0f, 180.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_BACK;
}

void CameraCtrl::leftSide() {
    doRotation(Vector3( 0.0f,-90.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_LEFT;
}

void CameraCtrl::rightSide() {
    doRotation(Vector3( 0.0f, 90.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_RIGHT;
}

void CameraCtrl::topSide() {
    doRotation(Vector3(-90.0f, 0.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_TOP;
}

void CameraCtrl::bottomSide() {
    doRotation(Vector3( 90.0f, 0.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_BOTTOM;
}

void CameraCtrl::doRotation(const Vector3 &vector) {
    m_rotation = vector;
    m_positionTarget = m_camera->transform()->position();

    m_cameraFreeSaved = m_cameraFree;
    m_cameraFree = false;
    m_transferProgress = 0.0f;
    m_rotationTransfer = true;
}

void CameraCtrl::onInputEvent(QInputEvent *pe) {
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
#ifdef Q_OS_LINUX
            quint32 key = e->key();
#else
            quint32 key = e->nativeVirtualKey();
#endif
            switch(key) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    if(!m_activeCamera->orthographic()) {
                        m_cameraMove |= MOVE_FORWARD;
                    }
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    if(!m_activeCamera->orthographic()) {
                        m_cameraMove |= MOVE_BACKWARD;
                    }
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    m_cameraMove |= MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    m_cameraMove |= MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::KeyRelease: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
#ifdef Q_OS_LINUX
            quint32 key = e->key();
#else
            quint32 key = e->nativeVirtualKey();
#endif
            switch(key) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    m_cameraMove &= ~MOVE_FORWARD;
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    m_cameraMove &= ~MOVE_BACKWARD;
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    m_cameraMove &= ~MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    m_cameraMove &= ~MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->buttons() & Qt::RightButton) {
                m_saved = e->globalPos();
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            QPoint pos = e->globalPos();
            QPoint delta = pos - m_saved;

            Vector3 p(static_cast<float>(delta.x()) / m_screenSize.x * m_activeCamera->ratio(),
                     -static_cast<float>(delta.y()) / m_screenSize.y, 0.0f);

            if(e->buttons() & Qt::RightButton) {
                if(m_activeCamera->orthographic()) {
                    Transform *t = m_camera->transform();
                    cameraMove(t->quaternion() * p);
                } else {
                    if(!m_blockRotation)  {
                        cameraRotate(Vector3(delta.y(), delta.x(), 0.0f) * 0.1f);
                    }
                }
            } else if(e->buttons() & Qt::MiddleButton) {
                Transform *t = m_camera->transform();
                cameraMove((t->quaternion() * p) * m_activeCamera->focal() * 0.1f);
            }
            m_saved = pos;
        } break;
        case QEvent::Wheel: {
            cameraZoom(static_cast<QWheelEvent *>(pe)->delta());
        } break;
        default: break;
    }
}

void CameraCtrl::cameraZoom(float delta) {
    if(m_activeCamera && m_camera) {
        if(m_activeCamera->orthographic()) {
            float scale = m_activeCamera->orthoSize() * 0.001f;
            m_activeCamera->setOrthoSize(MAX(0.0f, m_activeCamera->orthoSize() - delta * scale));
        } else {
            float scale = delta * 0.01f;
            float focal = m_activeCamera->focal() - scale;
            if(focal > 0.0f) {
                m_activeCamera->setFocal(focal);

                Transform *t = m_camera->transform();
                t->setPosition(t->position() - t->quaternion() * Vector3(0.0, 0.0, scale));
            }
        }
    }
}

void CameraCtrl::cameraRotate(const Vector3 &delta) {
    Transform *t = m_camera->transform();
    Vector3 euler = t->rotation() - delta;

    if(!m_cameraFree) {
        Vector3 dir = t->position() - t->quaternion() * Vector3(0.0, 0.0, m_activeCamera->focal());
        t->setPosition(dir + Quaternion(euler) * Vector3(0.0, 0.0, m_activeCamera->focal()));
    }
    t->setRotation(euler);
}

void CameraCtrl::cameraMove(const Vector3 &delta) {
    Transform *t = m_camera->transform();
    t->setPosition(t->position() - delta * ((m_activeCamera->orthographic()) ? m_activeCamera->orthoSize() : m_activeCamera->focal()));
}

bool CameraCtrl::restoreState(const VariantList &list) {
    bool result = false;
    if(m_activeCamera) {
        auto it = list.begin();
        Actor *actor = m_activeCamera->actor();
        Transform *t = actor->transform();
        t->setPosition(it->toVector3());
        it++;
        t->setRotation(it->toVector3());
        it++;
        result = it->toBool(); // ortho
        it++;
        m_activeCamera->setFocal(it->toFloat());
        it++;
        m_activeCamera->setOrthoSize(it->toFloat());
    }
    return result;
}

VariantList CameraCtrl::saveState() const {
    VariantList result;
    if(m_activeCamera) {
        Actor *actor = m_activeCamera->actor();
        Transform *t = actor->transform();
        result.push_back(t->position());
        result.push_back(t->rotation());
        result.push_back(m_activeCamera->orthographic());
        result.push_back(m_activeCamera->focal());
        result.push_back(m_activeCamera->orthoSize());
    }
    return result;
}

void CameraCtrl::setActiveRootObject(Object *object) {
    m_activeRootObject = object;
}

void CameraCtrl::drawHelpers(Object &object) {
    auto list = selected();

    for(auto &it : object.getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component) {
            if(component->drawHandles(list)) {
                select(object);
            }
        } else {
            if(it) {
                drawHelpers(*it);
            }
        }
    }
}
