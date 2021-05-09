#include "cameractrl.h"

#include <QMenu>

#include <engine.h>
#include <timer.h>
#include <float.h>

#include <components/scene.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/spriterender.h>
#include <components/textrender.h>

#include "graph/sceneview.h"

CameraCtrl::CameraCtrl(QOpenGLWidget *view) :
        m_CameraMove(MoveTypes::MOVE_IDLE),
        m_ViewSide(ViewSide::VIEW_SCENE),
        m_BlockMove(false),
        m_BlockRotation(false),
        m_CameraFree(true),
        m_CameraFreeSaved(true),
        m_RotationTransfer(false),
        m_OrthoWidthTarget(-1.0f),
        m_FocalLengthTarget(-1.0f),
        m_TransferProgress(1.0f),
        m_pCamera(nullptr),
        m_pView(view),
        m_pActiveCamera(nullptr) {
}

void CameraCtrl::init(Scene *scene) {
    Q_UNUSED(scene)
    m_pCamera = Engine::composeActor("Camera", "Camera", nullptr);
    m_pActiveCamera = static_cast<Camera *>(m_pCamera->component("Camera"));
    m_pActiveCamera->setFocal(10.0f);
    m_pActiveCamera->setOrthoSize(10.0f);

    m_pActiveCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0));

    m_pCamera->transform()->setPosition(Vector3(0.0, 0.0, 20.0));
}

void CameraCtrl::drawHandles() {

}

void CameraCtrl::resize(int32_t width, int32_t height) {
    Q_UNUSED(width)
    Q_UNUSED(height)
}

Object::ObjectList CameraCtrl::selected() {
    return Object::ObjectList();
}

void CameraCtrl::update() {
    if(!m_BlockMove) {
        if(m_CameraMove & MOVE_FORWARD) {
            m_CameraSpeed.z += 0.1f;
        }

        if(m_CameraMove & MOVE_BACKWARD) {
            m_CameraSpeed.z -= 0.1f;
        }

        if(m_CameraMove & MOVE_LEFT) {
            m_CameraSpeed.x -= 0.1f;
        }

        if(m_CameraMove & MOVE_RIGHT) {
            m_CameraSpeed.x += 0.1f;
        }
    }

    if(m_pActiveCamera) {
        Transform *t = m_pCamera->transform();
        if(m_TransferProgress < 1.0f) {
            if(m_RotationTransfer) {
                cameraRotate(t->rotation() - MIX(t->rotation(), m_RotationTarget, m_TransferProgress));
            } else {
                t->setPosition(MIX(t->position(), m_PositionTarget, m_TransferProgress));
            }

            if(m_OrthoWidthTarget > 0.0f) {
                m_pActiveCamera->setOrthoSize(MIX(m_pActiveCamera->orthoSize(), m_OrthoWidthTarget, m_TransferProgress));
            }

            if(m_FocalLengthTarget > 0.0f) {
                m_pActiveCamera->setFocal(MIX(m_pActiveCamera->focal(), m_FocalLengthTarget, m_TransferProgress));
            }

            m_TransferProgress += 2.0f * Timer::deltaTime();

            if(m_TransferProgress >= 1.0f) {
                m_CameraFree = m_CameraFreeSaved;

                if(m_RotationTransfer) {
                    cameraRotate(t->rotation() - m_RotationTarget);
                } else {
                    t->setPosition(m_PositionTarget);
                }

                if(m_OrthoWidthTarget > 0.0f) {
                    m_pActiveCamera->setOrthoSize(m_OrthoWidthTarget);
                }

                if(m_FocalLengthTarget > 0.0f) {
                    m_pActiveCamera->setFocal(m_FocalLengthTarget);
                }
            }
        }

        if(m_CameraSpeed.x != 0.0f || m_CameraSpeed.y != 0.0f || m_CameraSpeed.z != 0.0f) {
            Vector3 pos = t->position();
            Vector3 dir = t->quaternion() * Vector3(0.0, 0.0, 1.0);
            dir.normalize();

            Vector3 delta = (dir * m_CameraSpeed.z) + dir.cross(Vector3(0.0f, 1.0f, 0.0f)) * m_CameraSpeed.x;
            t->setPosition(pos - delta * m_pActiveCamera->focal() * 0.1f);

            m_CameraSpeed -= m_CameraSpeed * 10.0f * Timer::deltaTime();
            if(m_CameraSpeed.length() <= .01f) {
                m_CameraSpeed = Vector3();
            }
        }
    }
}

void CameraCtrl::onOrthographic(bool flag) {
    if(m_pActiveCamera->orthographic() != flag) {
        m_pActiveCamera->setOrthographic(flag);
        if(flag) {
            m_Rotation = m_pCamera->transform()->rotation();
            frontSide();
        } else {
            doRotation(m_Rotation);
        }
        m_OrthoWidthTarget = -1.0f;
        m_FocalLengthTarget = -1.0f;
    }
}

void CameraCtrl::setFocusOn(Actor *actor, float &bottom) {
    bottom  = 0;
    if(actor) {
        Transform *t = actor->transform();

        AABBox bb(Vector3(FLT_MAX), Vector3(-FLT_MAX));
        for(auto it : actor->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }
        float radius = (bb.extent.length() * 2.0f) / sinf(m_pActiveCamera->fov() * DEG2RAD);

        Vector3 min, max;
        bb.box(min, max);
        bottom = min.y;

        m_FocalLengthTarget = radius;
        m_OrthoWidthTarget = radius;
        Transform *camera = m_pCamera->transform();
        m_PositionTarget = t->worldPosition() + bb.center + camera->quaternion() * Vector3(0.0, 0.0, radius);
        m_TransferProgress = 0.0f;
        m_RotationTransfer = false;
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
    m_ViewSide = ViewSide::VIEW_FRONT;
}

void CameraCtrl::backSide() {
    doRotation(Vector3( 0.0f, 180.0f, 0.0f));
    m_ViewSide = ViewSide::VIEW_BACK;
}

void CameraCtrl::leftSide() {
    doRotation(Vector3( 0.0f,-90.0f, 0.0f));
    m_ViewSide = ViewSide::VIEW_LEFT;
}

void CameraCtrl::rightSide() {
    doRotation(Vector3( 0.0f, 90.0f, 0.0f));
    m_ViewSide = ViewSide::VIEW_RIGHT;
}

void CameraCtrl::topSide() {
    doRotation(Vector3(-90.0f, 0.0f, 0.0f));
    m_ViewSide = ViewSide::VIEW_TOP;
}

void CameraCtrl::bottomSide() {
    doRotation(Vector3( 90.0f, 0.0f, 0.0f));
    m_ViewSide = ViewSide::VIEW_BOTTOM;
}

void CameraCtrl::doRotation(const Vector3 &vector) {
    m_RotationTarget = vector;
    m_PositionTarget = m_pCamera->transform()->position();

    m_CameraFreeSaved = m_CameraFree;
    m_CameraFree = false;
    m_TransferProgress = 0.0f;
    m_RotationTransfer = true;
}

void CameraCtrl::onInputEvent(QInputEvent *pe) {
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    if(!m_pActiveCamera->orthographic()) {
                        m_CameraMove |= MOVE_FORWARD;
                    }
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    if(!m_pActiveCamera->orthographic()) {
                        m_CameraMove |= MOVE_BACKWARD;
                    }
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    m_CameraMove |= MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    m_CameraMove |= MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::KeyRelease: {
            QKeyEvent *e = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    m_CameraMove &= ~MOVE_FORWARD;
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    m_CameraMove &= ~MOVE_BACKWARD;
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    m_CameraMove &= ~MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    m_CameraMove &= ~MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            if(e->buttons() & Qt::RightButton) {
                m_Saved = e->globalPos();
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent *>(pe);
            QPoint pos = e->globalPos();
            QPoint delta = pos - m_Saved;

            Vector3 p(static_cast<float>(delta.x()) / (static_cast<float>(m_pView->width())),
                     -static_cast<float>(delta.y()) / (static_cast<float>(m_pView->height()) * m_pActiveCamera->ratio()), 0.0f);

            if(e->buttons() & Qt::RightButton) {
                if(m_pActiveCamera->orthographic()) {
                    Transform *t = m_pCamera->transform();
                    cameraMove(t->quaternion() * p);
                } else {
                    if(!m_BlockRotation)  {
                        cameraRotate(Vector3(delta.y(), delta.x(), 0.0f) * 0.1f);
                    }
                }
            } else if(e->buttons() & Qt::MiddleButton) {
                Transform *t = m_pCamera->transform();
                cameraMove((t->quaternion() * p) * m_pActiveCamera->focal() * 0.1f);
            }
            m_Saved = pos;
        } break;
        case QEvent::Wheel: {
            cameraZoom(static_cast<QWheelEvent *>(pe)->delta());
        } break;
        default: break;
    }
}

void CameraCtrl::cameraZoom(float delta) {
    if(m_pActiveCamera && m_pCamera) {
        if(m_pActiveCamera->orthographic()) {
            float scale = m_pActiveCamera->orthoSize() * 0.001f;
            m_pActiveCamera->setOrthoSize(MAX(0.0f, m_pActiveCamera->orthoSize() - delta * scale));
        } else {
            float scale = delta * 0.01f;
            float focal = m_pActiveCamera->focal() - scale;
            if(focal > 0.0f) {
                m_pActiveCamera->setFocal(focal);

                Transform *t = m_pCamera->transform();
                t->setPosition(t->position() - t->quaternion() * Vector3(0.0, 0.0, scale));
            }
        }
    }
}

void CameraCtrl::cameraRotate(const Vector3 &delta) {
    Transform *t = m_pCamera->transform();
    Vector3 euler = t->rotation() - delta;

    if(!m_CameraFree) {
        Vector3 dir = t->position() - t->quaternion() * Vector3(0.0, 0.0, m_pActiveCamera->focal());
        t->setPosition(dir + Quaternion(euler) * Vector3(0.0, 0.0, m_pActiveCamera->focal()));
    }
    t->setRotation(euler);
}

void CameraCtrl::cameraMove(const Vector3 &delta) {
    Transform *t = m_pCamera->transform();
    t->setPosition(t->position() - delta * ((m_pActiveCamera->orthographic()) ? m_pActiveCamera->orthoSize() : m_pActiveCamera->focal()));
}
