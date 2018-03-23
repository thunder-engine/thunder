#include "cameractrl.h"

#include <QApplication>
#include <QCursor>

#include <engine.h>
#include <timer.h>

#include <components/scene.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/staticmesh.h>

CameraCtrl::CameraCtrl(Engine *engine) :
        IController(engine),
        mBlockMove(false),
        mBlockRot(false),
        mBlockFree(false),
        mCameraFree(false),
        mCameraMove(MoveTypes::MOVE_IDLE),
        mCameraSpeed(Vector3()),
        mRotation(Vector3()),
        m_pCamera(nullptr) {
}

void CameraCtrl::init(Scene *scene) {
    m_pCamera   = Engine::objectCreate<Actor>("Camera", scene);
    m_pCamera->setScene(*scene);
    m_pActiveCamera = m_pCamera->addComponent<Camera>();
    m_pActiveCamera->setFocal(20.0f);
    //m_pActiveCamera->setType(Camera::ORTHOGRAPHIC);

    m_pCamera->setPosition(Vector3(0.0, 0.0, 20.0));
}

void CameraCtrl::update() {
    if(!mBlockMove) {
        if(mCameraMove & MOVE_FORWARD) {
            mCameraSpeed.z += 0.5f;
        }

        if(mCameraMove & MOVE_BACKWARD) {
            mCameraSpeed.z -= 0.5f;
        }

        if(mCameraMove & MOVE_LEFT) {
            mCameraSpeed.x -= 0.5f;
        }

        if(mCameraMove & MOVE_RIGHT) {
            mCameraSpeed.x += 0.5f;
        }
    }

    if( m_pActiveCamera && (mCameraSpeed.x != 0 || mCameraSpeed.y != 0 || mCameraSpeed.z != 0) ) {
        Vector3 pos   = m_pCamera->position();
        Vector3 dir   = m_pCamera->rotation() * Vector3(0.0, 0.0, 1.0);
        dir.normalize();

        Vector3 delta = (dir * mCameraSpeed.z) + dir.cross(Vector3(0.0f, 1.0f, 0.0f)) * mCameraSpeed.x;
        m_pCamera->setPosition(pos - delta);

        mCameraSpeed   -= mCameraSpeed * 10.0f * Timer::deltaTime();
        if(mCameraSpeed.length() <= .01f) {
            mCameraSpeed    = Vector3();
        }
    }
}

void CameraCtrl::drawHandles() {

}

void CameraCtrl::setFocusOn(Actor *actor, float &bottom) {
    bottom          = 0;
    if(actor) {
        Vector3 pos;
        float radius    = 0;
        for(auto it : actor->getChildren()) {
            StaticMesh *staticMesh  = dynamic_cast<StaticMesh *>(it);
            if(staticMesh && staticMesh->mesh()) {
                uint32_t i  = 0;
                for(uint32_t s = 0; s < staticMesh->mesh()->surfacesCount(); s++) {
                    AABBox aabb = staticMesh->mesh()->bound(s);
                    pos += aabb.center;
                    radius += aabb.size.length();
                    Vector3 min, max;
                    aabb.box(min, max);
                    if(i == 0) {
                        bottom  = min.y;
                    }
                    bottom = MIN(bottom, min.y);
                    i++;
                }
                uint32_t size   = staticMesh->mesh()->surfacesCount();
                pos /= size;
                radius /= size;
                radius /= sinf(m_pActiveCamera->fov() * DEG2RAD);
            }
        }

        m_pActiveCamera->setFocal(radius);
        m_pCamera->setPosition(actor->position() + pos + m_pCamera->rotation() * Vector3(0.0, 0.0, radius));
    }
}

void CameraCtrl::onInputEvent(QInputEvent *pe) {
    switch(pe->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *e    = static_cast<QKeyEvent *>(pe);
            switch(e->key()) {
                case Qt::Key_W:
                case Qt::Key_Up: {
                    mCameraMove |= MOVE_FORWARD;
                } break;
                case Qt::Key_S:
                case Qt::Key_Down: {
                    mCameraMove |= MOVE_BACKWARD;
                } break;
                case Qt::Key_A:
                case Qt::Key_Left: {
                    mCameraMove |= MOVE_LEFT;
                } break;
                case Qt::Key_D:
                case Qt::Key_Right: {
                    mCameraMove |= MOVE_RIGHT;
                } break;
                case Qt::Key_Alt: {
                    if(!mBlockFree) {
                        mCameraFree = true;
                    }
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
                case Qt::Key_Alt: {
                    if(!mBlockFree) {
                        mCameraFree = false;
                    }
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
            QPoint d        = e->globalPos() - mSaved;
            if(!mBlockRot && e->buttons() & Qt::RightButton) {
                mRotation.x -= (float)d.x() / 10.0;
                mRotation.y -= (float)d.y() / 10.0;
                cameraRotate(Vector3(CLAMP(mRotation.y, -80.0f, 80.0f), mRotation.x, 0.0));

                QCursor::setPos(mSaved);
            }
        } break;
        case QEvent::Wheel: {
            cameraZoom(static_cast<QWheelEvent *>(pe)->delta() / 100.0f);
        } break;
        default: break;
    }
}

void CameraCtrl::cameraZoom(float delta) {
    if(m_pActiveCamera && m_pCamera) {
        if(m_pActiveCamera->type() == Camera::ORTHOGRAPHIC) {
            double width = m_pActiveCamera->orthoWidth() - delta / 10.0f;
            if(width > 0.0) {
                m_pActiveCamera->setOrthoWidth(width);
            }
        } else {
            m_pActiveCamera->setFocal(m_pActiveCamera->focal() - delta);
            Vector3 pos   = m_pCamera->position() - m_pCamera->rotation() * Vector3(0.0, 0.0, 1.0) * delta;
            m_pCamera->setPosition(pos);
        }
    }
}

void CameraCtrl::cameraRotate(const Quaternion  &q) {
    Vector3 target = m_pCamera->position() - m_pCamera->rotation() * Vector3(0.0, 0.0, m_pActiveCamera->focal());
    if(!mCameraFree) {
        m_pCamera->setPosition(target + q * Vector3(0.0, 0.0, m_pActiveCamera->focal()));
    }
    m_pCamera->setRotation(q);
}
