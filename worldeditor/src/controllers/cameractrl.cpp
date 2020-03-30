#include "cameractrl.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCursor>

#include <engine.h>
#include <timer.h>

#include <components/scene.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/spriterender.h>
#include <components/textrender.h>

#include "graph/sceneview.h"

CameraCtrl::CameraCtrl(QOpenGLWidget *view) :
        mCameraMove(MoveTypes::MOVE_IDLE),
        mCameraFree(true),
        mBlockMove(false),
        mBlockRot(false),
        mCameraSpeed(Vector3()),
        m_pCamera(nullptr),
        m_pView(view),
        m_pActiveCamera(nullptr) {
}

void CameraCtrl::init(Scene *scene) {
    Q_UNUSED(scene)
    m_pCamera   = Engine::objectCreate<Actor>("Camera");
    m_pActiveCamera = static_cast<Camera *>(m_pCamera->addComponent("Camera"));
    m_pActiveCamera->setFocal(10.0f);
    m_pActiveCamera->setOrthoSize(10.0f);

    m_pActiveCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0));

    m_pCamera->transform()->setPosition(Vector3(0.0, 0.0, 20.0));
}

void CameraCtrl::update() {
    if(!mBlockMove) {
        if(mCameraMove & MOVE_FORWARD) {
            mCameraSpeed.z += 0.1f;
        }

        if(mCameraMove & MOVE_BACKWARD) {
            mCameraSpeed.z -= 0.1f;
        }

        if(mCameraMove & MOVE_LEFT) {
            mCameraSpeed.x -= 0.1f;
        }

        if(mCameraMove & MOVE_RIGHT) {
            mCameraSpeed.x += 0.1f;
        }
    }

    if( m_pActiveCamera && (mCameraSpeed.x != 0.0f || mCameraSpeed.y != 0.0f || mCameraSpeed.z != 0.0f) ) {
        Transform *t    = m_pCamera->transform();
        Vector3 pos     = t->position();
        Vector3 dir     = t->rotation() * Vector3(0.0, 0.0, 1.0);
        dir.normalize();

        Vector3 delta = (dir * mCameraSpeed.z) + dir.cross(Vector3(0.0f, 1.0f, 0.0f)) * mCameraSpeed.x;
        t->setPosition(pos - delta * m_pActiveCamera->focal() * 0.1f);

        mCameraSpeed   -= mCameraSpeed * 10.0f * Timer::deltaTime();
        if(mCameraSpeed.length() <= .01f) {
            mCameraSpeed    = Vector3();
        }
    }
}

void CameraCtrl::onOrthographic(bool flag) {
    if(m_pActiveCamera->orthographic() != flag) {
        Transform *t    = m_pCamera->transform();
        if(flag) {
            mRotation   = t->rotation();
            t->setRotation(Quaternion());
        } else {
            t->setRotation(mRotation);
        }

        m_pActiveCamera->setOrthographic(flag);
    }
}

void CameraCtrl::setFocusOn(Actor *actor, float &bottom) {
    bottom  = 0;
    if(actor) {

        Transform *t = actor->transform();

        /// \todo Encapsulation may go wrong in case of all points on the one side of axis
        AABBox bb(Vector3(0.0f), Vector3(0.0f));
        for(auto it : actor->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }
        float radius = (bb.extent.length() * 2) / sinf(m_pActiveCamera->fov() * DEG2RAD);

        Vector3 min, max;
        bb.box(min, max);
        bottom = min.y;

        m_pActiveCamera->setFocal(radius);
        m_pActiveCamera->setOrthoSize(radius);
        Transform *camera   = m_pCamera->transform();
        camera->setPosition(t->worldPosition() + bb.center + camera->rotation() * Vector3(0.0, 0.0, radius));
    }
}

void CameraCtrl::onInputEvent(QInputEvent *pe) {
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e    = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    if(!m_pActiveCamera->orthographic()) {
                        mCameraMove |= MOVE_FORWARD;
                    }
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    if(!m_pActiveCamera->orthographic()) {
                        mCameraMove |= MOVE_BACKWARD;
                    }
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    mCameraMove |= MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    mCameraMove |= MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::KeyRelease: {
            QKeyEvent *e    = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    mCameraMove &= ~MOVE_FORWARD;
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    mCameraMove &= ~MOVE_BACKWARD;
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    mCameraMove &= ~MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    mCameraMove &= ~MOVE_RIGHT;
                } break;
                default: break;
            }
        } break;
        case QEvent::MouseButtonPress: {
            QMouseEvent *e  = static_cast<QMouseEvent *>(pe);
            if(e->buttons() & Qt::RightButton) {
                mSaved  = e->globalPos();
            }
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e  = static_cast<QMouseEvent *>(pe);
            QPoint pos      = e->globalPos();
            QPoint delta    = pos - mSaved;

            if(e->buttons() & Qt::RightButton) {
                if(m_pActiveCamera->orthographic()) {
                    cameraMove(Vector3(static_cast<float>(delta.x()) / (static_cast<float>(m_pView->width())),
                                      -static_cast<float>(delta.y()) / (static_cast<float>(m_pView->height()) * m_pActiveCamera->ratio()), 0.0f));
                } else {
                    if(!mBlockRot)  {
                        cameraRotate(Vector3(delta.y(), delta.x(), 0.0f) * 0.1f);
                    }
                }

                QRect r = QApplication::desktop()->screenGeometry(m_pView);
                if(pos.x() >= r.right()) {
                    pos.setX(pos.x() - r.width());
                }
                if(pos.x() <= r.left()) {
                    pos.setX(pos.x() + r.width());
                }

                if(pos.y() >= r.bottom()) {
                    pos.setY(pos.y() - r.height() + 2);
                }
                if(pos.y() <= r.top()) {
                    pos.setY(pos.y() + r.height() - 2);
                }
                QCursor::setPos(pos);
            }
            mSaved  = pos;
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
                t->setPosition(t->position() - t->rotation() * Vector3(0.0, 0.0, scale));
            }
        }
    }
}

void CameraCtrl::cameraRotate(const Vector3 &delta) {
    Transform *t  = m_pCamera->transform();
    Vector3 euler = t->euler() - delta;
    mRotation     = Quaternion(euler);

    Vector3 target  = t->position() - t->rotation() * Vector3(0.0, 0.0, m_pActiveCamera->focal());
    if(!mCameraFree) {
        t->setPosition(target + mRotation * Vector3(0.0, 0.0, m_pActiveCamera->focal()));
    }
    t->setEuler(euler);
}

void CameraCtrl::cameraMove(const Vector3 &delta) {
    Transform *t = m_pCamera->transform();
    t->setPosition(t->position() - delta * ((m_pActiveCamera->orthographic()) ? m_pActiveCamera->orthoSize() : m_pActiveCamera->focal()));
}
