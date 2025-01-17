#include "editor/viewport/cameracontroller.h"
#include "editor/viewport/handles.h"
#include "editor/viewport/handletools.h"

#include <QMenu>

#include <float.h>

#include <engine.h>
#include <input.h>

#include <components/world.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/meshrender.h>
#include <components/spriterender.h>
#include <components/textrender.h>

#define DT 0.0625f

const float s_Sence = 0.04f;

namespace {
    const char *gCamera("Camera");
    const char *gPosition("position");
    const char *gRotation("rotation");
    const char *gOrthographic("orthographic");
    const char *gFocal("focal");
    const char *gOrthoSize("orthoSize");
    const char *gGridAxis("gridAxis");
}

CameraController::CameraController() :
        m_gridAxis(Axis::X),
        m_blockMove(false),
        m_blockRotation(false),
        m_cameraFree(true),
        m_cameraFreeSaved(true),
        m_rotationTransfer(false),
        m_cameraInMove(false),
        m_allowPicking(true),
        m_orthoWidthTarget(-1.0f),
        m_focalLengthTarget(-1.0f),
        m_transferProgress(1.0f),
        m_activeCamera(nullptr),
        m_activeRootObject(nullptr),
        m_zoomLimit(0.001f, 10000.0f) {

    Actor *actor = Engine::composeActor(gCamera, gCamera, nullptr);
    m_activeCamera = static_cast<Camera *>(actor->component(gCamera));
    m_activeCamera->setFocal(10.0f);
    m_activeCamera->setOrthoSize(10.0f);

    m_activeCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0f));

    m_activeCamera->transform()->setPosition(Vector3(0.0f, 0.0f, 10.0f));
}

void CameraController::drawHandles() {
    drawHelpers(m_activeRootObject);
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
    m_delta = Vector2(pos.x, pos.y) - m_saved;

    Vector3 p(m_delta.x / m_screenSize.x * m_activeCamera->ratio(), m_delta.y / m_screenSize.y, 0.0f);

    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(Input::isMouseButton(Input::MOUSE_RIGHT)) {
        if(m_activeCamera->orthographic()) {
            if(!m_blockMove) {
                Transform *t = m_activeCamera->transform();
                cameraMove(t->quaternion() * p);

                Vector2 p(pos.x, pos.y);

                if(m_saved != p) {
                    m_cameraInMove = true;
                }
                m_saved = p;
            }
        } else if(!m_blockRotation)  {
            cameraRotate(Vector3(-m_delta.y, m_delta.x, 0.0f) * 0.1f);
            m_saved = Vector2(pos.x, pos.y);
        }
    } else if(Input::isMouseButton(Input::MOUSE_MIDDLE) && !m_blockMove) {
        Transform *t = m_activeCamera->transform();
        float mult = m_activeCamera->orthographic() ? 1.0f : m_activeCamera->focal() * 0.1f;
        cameraMove((t->quaternion() * p) * mult);
        m_saved = Vector2(pos.x, pos.y);
    }

    // Camera zoom
    float delta = Input::mouseScrollDelta();
    if(delta != 0.0f) {
        cameraZoom(delta);
    }
}

void CameraController::move() {
    // Linear movements
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
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
            m_rotation = m_activeCamera->transform()->rotation();
            // Front
            doRotation(Vector3());
            setGridAxis(Axis::Z);
        } else {
            setGridAxis(Axis::Y);
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
            Transform *camera = m_activeCamera->transform();
            m_positionTarget = bb.center + camera->quaternion() * Vector3(0.0f, 0.0f, radius);
            m_transferProgress = 0.0f;
            m_rotationTransfer = false;
        }
    }
}

void CameraController::doRotation(const Vector3 &vector) {
    m_rotation = vector;
    m_positionTarget = m_activeCamera->transform()->position();

    m_cameraFreeSaved = m_cameraFree;
    m_cameraFree = false;
    m_transferProgress = 0.0f;
    m_rotationTransfer = true;
}

void CameraController::cameraZoom(float delta) {
    if(m_activeCamera) {
        if(m_activeCamera->orthographic()) {
            float size = m_activeCamera->orthoSize();

            float scale = size / m_screenSize.x;

            scale = CLAMP(size - (delta * scale), m_zoomLimit.x, m_zoomLimit.y);

            m_activeCamera->setOrthoSize(scale);
        } else {
            float scale = delta * 0.01f;
            float focal = CLAMP(m_activeCamera->focal() - scale, m_zoomLimit.x, m_zoomLimit.y);

            m_activeCamera->setFocal(focal);

            Transform *t = m_activeCamera->transform();
            t->setPosition(t->position() - t->quaternion() * Vector3(0.0f, 0.0f, scale));
        }
    }
}

void CameraController::cameraRotate(const Vector3 &delta) {
    Transform *t = m_activeCamera->transform();
    Vector3 euler = t->rotation() - delta;

    if(!m_cameraFree) {
        Vector3 dir = t->position() - t->quaternion() * Vector3(0.0f, 0.0f, m_activeCamera->focal());
        t->setPosition(dir + Quaternion(euler) * Vector3(0.0f, 0.0f, m_activeCamera->focal()));
    }
    t->setRotation(euler);
}

void CameraController::cameraMove(const Vector3 &delta) {
    Transform *t = m_activeCamera->transform();
    t->setPosition(t->position() - delta * ((m_activeCamera->orthographic()) ? m_activeCamera->orthoSize() : m_activeCamera->focal()));
}

void CameraController::restoreState(const VariantMap &state) {
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
        auto it = state.find(gPosition);
        if(it != state.end()) {
            t->setPosition((*it).second.toVector3());
        }

        it = state.find(gRotation);
        if(it != state.end()) {
            t->setRotation((*it).second.toVector3());
        }

        it = state.find(gOrthographic);
        if(it != state.end()) {
            m_activeCamera->setOrthographic((*it).second.toBool());
        }

        it = state.find(gFocal);
        if(it != state.end()) {
            m_activeCamera->setFocal((*it).second.toFloat());
        }

        it = state.find(gOrthoSize);
        if(it != state.end()) {
            m_activeCamera->setOrthoSize((*it).second.toFloat());
        }

        it = state.find(gGridAxis);
        if(it != state.end()) {
            m_gridAxis = static_cast<Axis>((*it).second.toInt());
        }
    }
}

VariantMap CameraController::saveState() const {
    VariantMap result;
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
        result[gPosition] = t->position();
        result[gRotation] = t->rotation();
        result[gOrthographic] = m_activeCamera->orthographic();
        result[gFocal] = m_activeCamera->focal();
        result[gOrthoSize] = m_activeCamera->orthoSize();
        result[gGridAxis] = static_cast<int>(m_gridAxis);
    }
    return result;
}

void CameraController::setActiveRootObject(Object *object) {
    m_activeRootObject = object;
}

void CameraController::setZoomLimits(const Vector2 &limit) {
    m_zoomLimit = limit;
}

void CameraController::drawHelpers(Object *object) {
    bool sel = selected().contains(object);

    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        if(!actor->isEnabled()) {
            return;
        }
        bool isRenderable = false;
        for(auto &it : actor->getChildren()) {
            Actor *actor = dynamic_cast<Actor *>(it);
            if(actor) {
                drawHelpers(actor);
            } else {
                Component *component = dynamic_cast<Component *>(it);
                if(component) {
                    component->drawGizmos();

                    if(sel) {
                        component->drawGizmosSelected();
                    }

                    Renderable *renderable = dynamic_cast<Renderable *>(it);
                    if(renderable) {
                        isRenderable = true;
                    }
                }
            }
        }

        if(!isRenderable) {
            Transform *t = actor->transform();
            if(t) {
                float distance = HandleTools::distanceToPoint(Matrix4(), t->worldPosition(), Handles::s_Mouse);
                if(distance <= s_Sence) {
                    select(*actor);
                }
            }
        }
    } else {
        for(auto &it : object->getChildren()) {
            drawHelpers(it);
        }
    }
}
