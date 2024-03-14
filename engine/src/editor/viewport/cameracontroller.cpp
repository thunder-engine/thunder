#include "editor/viewport/cameracontroller.h"
#include "editor/viewport/handles.h"
#include "editor/viewport/handletools.h"

#include <QMenu>

#include <float.h>

#include <engine.h>
#include <timer.h>
#include <input.h>

#include <components/world.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/spriterender.h>
#include <components/textrender.h>

#define DT 0.0625f

const char *gCamera("Camera");

CameraController::CameraController() :
        m_viewSide(ViewSide::VIEW_SCENE),
        m_blockMove(false),
        m_blockRotation(false),
        m_cameraFree(true),
        m_cameraFreeSaved(true),
        m_rotationTransfer(false),
        m_cameraInMove(false),
        m_orthoWidthTarget(-1.0f),
        m_focalLengthTarget(-1.0f),
        m_transferProgress(1.0f),
        m_camera(nullptr),
        m_activeCamera(nullptr),
        m_activeRootObject(nullptr),
        m_zoomLimit(0.001f, 10000.0f) {

    m_camera = Engine::composeActor(gCamera, gCamera, nullptr);
    m_activeCamera = static_cast<Camera *>(m_camera->component(gCamera));
    m_activeCamera->setFocal(10.0f);
    m_activeCamera->setOrthoSize(10.0f);

    m_activeCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0));

    m_camera->transform()->setPosition(Vector3(0.0, 0.0, 20.0));
}

void CameraController::drawHandles() {
    drawHelpers(*m_activeRootObject);
}

void CameraController::resize(int32_t width, int32_t height) {
    m_screenSize = Vector2(width, height);
}

QList<Object *> CameraController::selected() {
    return QList<Object *>();
}

void CameraController::select(Object &object) {

}

void CameraController::update() {
    if(!m_blockMove) {
        if(Input::isKey(Input::KEY_W) || Input::isKey(Input::KEY_UP)) {
            if(!m_activeCamera->orthographic()) {
                m_cameraSpeed.z += 0.1f;
            }
        }

        if(Input::isKey(Input::KEY_S) || Input::isKey(Input::KEY_DOWN)) {
            if(!m_activeCamera->orthographic()) {
                m_cameraSpeed.z -= 0.1f;
            }
        }

        if(Input::isKey(Input::KEY_A) || Input::isKey(Input::KEY_LEFT)) {
            m_cameraSpeed.x -= 0.1f;
        }

        if(Input::isKey(Input::KEY_D) || Input::isKey(Input::KEY_RIGHT)) {
            m_cameraSpeed.x += 0.1f;
        }
    }

    Vector4 pos(Input::mousePosition());
    if(Input::isMouseButtonDown(Input::MOUSE_RIGHT) || Input::isMouseButtonDown(Input::MOUSE_MIDDLE)) {
        m_saved = Vector2(pos.x, pos.y);
    }

    if(Input::isMouseButtonUp(Input::MOUSE_RIGHT) || Input::isMouseButtonUp(Input::MOUSE_MIDDLE)) {
        m_cameraInMove = false;
    }

    // Mouse control
    Vector2 delta = Vector2(pos.x, pos.y) - m_saved;

    Vector3 p(delta.x / m_screenSize.x * m_activeCamera->ratio(), delta.y / m_screenSize.y, 0.0f);

    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(Input::isMouseButton(Input::MOUSE_RIGHT)) {
        if(m_activeCamera->orthographic()) {
            if(!m_blockMove) {
                Transform *t = m_camera->transform();
                cameraMove(t->quaternion() * p);

                Vector2 p(pos.x, pos.y);

                if(m_saved != p) {
                    m_cameraInMove = true;
                }
                m_saved = p;
            }
        } else if(!m_blockRotation)  {
            cameraRotate(Vector3(-delta.y, delta.x, 0.0f) * 0.1f);
            m_saved = Vector2(pos.x, pos.y);
        }
    } else if(Input::isMouseButton(Input::MOUSE_MIDDLE) && !m_blockMove) {
        Transform *t = m_camera->transform();
        cameraMove((t->quaternion() * p) * m_activeCamera->focal() * 0.1f);
        m_saved = Vector2(pos.x, pos.y);
    }

    // Camera zoom
    cameraZoom(Input::mouseScrollDelta());
}

void CameraController::move() {
    // Linear movements
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
            Vector3 dir = t->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
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

void CameraController::onOrthographic(bool flag) {
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

void CameraController::setFocusOn(Actor *actor, float &bottom) {
    bottom  = 0;
    if(actor) {
        AABBox bb(Vector3(FLT_MAX), Vector3(-FLT_MAX));
        for(auto it : actor->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }

        if(bb.isValid()) {
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
}

void CameraController::createMenu(QMenu *menu) {
    if(menu) {
        menu->addAction(tr("Front View"), this, SLOT(frontSide()));
        menu->addAction(tr("Back View"), this, SLOT(backSide()));
        menu->addAction(tr("Left View"), this, SLOT(leftSide()));
        menu->addAction(tr("Right View"), this, SLOT(rightSide()));
        menu->addAction(tr("Top View"), this, SLOT(topSide()));
        menu->addAction(tr("Bottom View"), this, SLOT(bottomSide()));
    }
}

void CameraController::frontSide() {
    doRotation(Vector3());
    m_viewSide = ViewSide::VIEW_FRONT;
}

void CameraController::backSide() {
    doRotation(Vector3( 0.0f, 180.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_BACK;
}

void CameraController::leftSide() {
    doRotation(Vector3( 0.0f,-90.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_LEFT;
}

void CameraController::rightSide() {
    doRotation(Vector3( 0.0f, 90.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_RIGHT;
}

void CameraController::topSide() {
    doRotation(Vector3(-90.0f, 0.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_TOP;
}

void CameraController::bottomSide() {
    doRotation(Vector3( 90.0f, 0.0f, 0.0f));
    m_viewSide = ViewSide::VIEW_BOTTOM;
}

void CameraController::doRotation(const Vector3 &vector) {
    m_rotation = vector;
    m_positionTarget = m_camera->transform()->position();

    m_cameraFreeSaved = m_cameraFree;
    m_cameraFree = false;
    m_transferProgress = 0.0f;
    m_rotationTransfer = true;
}

void CameraController::cameraZoom(float delta) {
    if(m_activeCamera && m_camera) {
        if(m_activeCamera->orthographic()) {
            float scale = m_activeCamera->orthoSize() * 0.001f;
            m_activeCamera->setOrthoSize(CLAMP(m_activeCamera->orthoSize() - delta * scale, m_zoomLimit.x, m_zoomLimit.y));
        } else {
            float scale = delta * 0.01f;
            float focal = CLAMP(m_activeCamera->focal() - scale, m_zoomLimit.x, m_zoomLimit.y);
            if(focal > 0.0f) {
                m_activeCamera->setFocal(focal);

                Transform *t = m_camera->transform();
                t->setPosition(t->position() - t->quaternion() * Vector3(0.0, 0.0, scale));
            }
        }
    }
}

void CameraController::cameraRotate(const Vector3 &delta) {
    Transform *t = m_camera->transform();
    Vector3 euler = t->rotation() - delta;

    if(!m_cameraFree) {
        Vector3 dir = t->position() - t->quaternion() * Vector3(0.0, 0.0, m_activeCamera->focal());
        t->setPosition(dir + Quaternion(euler) * Vector3(0.0, 0.0, m_activeCamera->focal()));
    }
    t->setRotation(euler);
}

void CameraController::cameraMove(const Vector3 &delta) {
    Transform *t = m_camera->transform();
    t->setPosition(t->position() - delta * ((m_activeCamera->orthographic()) ? m_activeCamera->orthoSize() : m_activeCamera->focal()));
}

void CameraController::restoreState(const VariantMap &state) {
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
        auto it = state.find("position");
        if(it != state.end()) {
            t->setPosition((*it).second.toVector3());
        }

        it = state.find("rotation");
        if(it != state.end()) {
            t->setRotation((*it).second.toVector3());
        }

        it = state.find("orthographic");
        if(it != state.end()) {
            m_activeCamera->setOrthographic((*it).second.toBool());
        }

        it = state.find("focal");
        if(it != state.end()) {
            m_activeCamera->setFocal((*it).second.toFloat());
        }

        it = state.find("orthoSize");
        if(it != state.end()) {
            m_activeCamera->setOrthoSize((*it).second.toFloat());
        }
    }
}

VariantMap CameraController::saveState() const {
    VariantMap result;
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
        result["position"] = t->position();
        result["rotation"] = t->rotation();
        result["orthographic"] = m_activeCamera->orthographic();
        result["focal"] = m_activeCamera->focal();
        result["orthoSize"] = m_activeCamera->orthoSize();
    }
    return result;
}

void CameraController::setActiveRootObject(Object *object) {
    m_activeRootObject = object;
}

void CameraController::setZoomLimits(const Vector2 &limit) {
    m_zoomLimit = limit;
}

void CameraController::drawHelpers(Object &object) {
    auto list = selected();

    for(auto &it : object.getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component && component->actor()->isEnabled()) {
            component->drawGizmos();
            float distance = HandleTools::distanceToPoint(Matrix4(), component->transform()->worldPosition(), Handles::s_Mouse);
            if(distance <= HandleTools::s_Sense) {
                select(object);
            }

            for(auto sel : list) {
                if(component->actor() == sel) {
                    component->drawGizmosSelected();
                }
            }
        } else {
            if(it) {
                drawHelpers(*it);
            }
        }
    }
}
