#include "editor/viewport/cameracontroller.h"
#include "editor/viewport/handles.h"
#include "editor/viewport/handletools.h"

#include <float.h>

#include <engine.h>
#include <input.h>
#include <timer.h>

#include <components/world.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

namespace {
    const char *gCamera("Camera");

    const char *gCameras("cameras");
    const char *gCameraIndex("cameraIndex");

    const char *gPosition("position");
    const char *gRotation("rotation");
    const char *gOrthographic("orthographic");
    const char *gFocal("focal");
    const char *gOrthoSize("orthoSize");
    const char *gGridAxis("gridAxis");
}

CameraController::CameraController() :
        m_activeCamera(nullptr),
        m_activeRootObject(nullptr),
        m_gridAxis(Axis::X),
        m_transferProgress(1.0f),
        m_currentCamera(-1),
        m_zoomLimit(0.001f, 10000.0f),
        m_blockMove(false),
        m_blockMoveOnTransfer(false),
        m_blockRotation(false),
        m_blockPicking(false),
        m_overlapPicking(false),
        m_cameraFree(true),
        m_cameraInMove(false) {

    Actor *actor = Engine::composeActor<Camera>(gCamera);
    m_activeCamera = actor->getComponent<Camera>();
    m_activeCamera->setColor(Vector4(0.2f, 0.2f, 0.2f, 0.0f));

    resetCamera();
}

void CameraController::drawHandles() {
    if(m_activeRootObject) {
        drawHelpers(m_activeRootObject, false);

        for(auto it : selected()) {
            drawHelpers(it, true);
        }
    }
}

void CameraController::resize(int32_t width, int32_t height) {
    m_screenSize = Vector2(width, height);
}

Object::ObjectList CameraController::selected() {
    return Object::ObjectList();
}

void CameraController::select(Object &object) {
    A_UNUSED(object);
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

        if(Input::isKey(Input::KEY_Q)) {
            m_cameraSpeed.y -= 0.1f;
        }

        if(Input::isKey(Input::KEY_E)) {
            m_cameraSpeed.y += 0.1f;
        }
    }

    Vector4 pos(Input::mousePosition());

    if(Input::isMouseButtonDown(Input::MOUSE_RIGHT) || Input::isMouseButtonDown(Input::MOUSE_MIDDLE)) {
        m_mouseSaved = Vector2(pos.x, pos.y);
    }

    if(Input::isMouseButtonUp(Input::MOUSE_RIGHT) || Input::isMouseButtonUp(Input::MOUSE_MIDDLE)) {
        m_cameraInMove = false;
    }

    // Mouse control
    m_mouseDelta = Vector2(pos.x - m_mouseSaved.x, pos.y - m_mouseSaved.y);

    Vector3 p(m_mouseDelta.x / m_screenSize.x * m_activeCamera->ratio(), m_mouseDelta.y / m_screenSize.y, 0.0f);

    Handles::s_Mouse = Vector2(pos.z, pos.w);
    Handles::s_Screen = m_screenSize;

    if(Input::isMouseButton(Input::MOUSE_RIGHT)) {
        if(m_activeCamera->orthographic()) {
            if(!m_blockMove) {
                Transform *t = m_activeCamera->transform();
                cameraMove(t->quaternion() * p);

                Vector2 p(pos.x, pos.y);

                if(m_mouseSaved != p) {
                    m_cameraInMove = true;
                }
                m_mouseSaved = p;
            }
        } else if(!m_blockRotation)  {
            cameraRotate(Vector3(-m_mouseDelta.y, m_mouseDelta.x, 0.0f) * 0.1f);
            m_mouseSaved = Vector2(pos.x, pos.y);
        }
    } else if(Input::isMouseButton(Input::MOUSE_MIDDLE) && !m_blockMove) {
        Transform *t = m_activeCamera->transform();
        float mult = m_activeCamera->orthographic() ? 1.0f : m_activeCamera->focalDistance() * 0.1f;
        cameraMove((t->quaternion() * p) * mult);
        m_mouseSaved = Vector2(pos.x, pos.y);
    }

    // Camera zoom
    float scrollDelta = Input::mouseScrollDelta();
    if(scrollDelta != 0.0f) {
        cameraZoom(scrollDelta);
    }
}

void CameraController::move() {
    // Linear movements
    if(m_activeCamera) {
        Transform *t = m_activeCamera->transform();
        if(m_transferProgress < 1.0f) {
            m_transferProgress = CLAMP(m_transferProgress + 2.0f * Timer::deltaTime(), 0.0f, 1.0f);

            if(!m_blockMoveOnTransfer) {
                t->setPosition(MIX(t->position(), m_targetCamera.position, m_transferProgress));
            }

            Vector3 euler(MIX(t->rotation(), m_targetCamera.rotation, m_transferProgress));

            if(!m_cameraFree || m_blockMoveOnTransfer) {
                Vector3 forward(0.0f, 0.0f, m_activeCamera->focalDistance());
                Vector3 dir(t->position() - t->quaternion() * forward);

                t->setPosition(dir + Quaternion(euler) * forward);
            }
            t->setRotation(euler);

            m_activeCamera->setOrthoSize(MIX(m_activeCamera->orthoSize(), m_targetCamera.orthoSize, m_transferProgress));
            m_activeCamera->setFocalDistance(MIX(m_activeCamera->focalDistance(), m_targetCamera.focalDistance, m_transferProgress));

            if(m_transferProgress >= 1.0f) {
                m_blockMoveOnTransfer = false;
            }
        }

        if(m_cameraSpeed.length() != 0.0f) {
            Vector3 pos = t->position();
            Vector3 dir = t->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
            dir.normalize();

            Vector3 delta = (dir * m_cameraSpeed.z);
            delta += dir.cross(Vector3(0.0f, 1.0f, 0.0f)) * m_cameraSpeed.x;
            delta += Vector3(0.0f, 1.0f, 0.0f) * m_cameraSpeed.y;

            t->setPosition(pos - delta * m_activeCamera->focalDistance() * 0.1f);

            m_cameraSpeed -= m_cameraSpeed * 8.0f * Timer::deltaTime();
            if(m_cameraSpeed.length() <= .01f) {
                m_cameraSpeed = Vector3();
            }
        }
    }
}

void CameraController::onOrthographic(bool flag) {
    m_activeCamera->setOrthographic(flag);
}

void CameraController::doRotation(const Vector3 &rotation) {
    m_targetCamera.rotation = rotation;
    m_targetCamera.focalDistance = m_activeCamera->focalDistance();
    m_targetCamera.orthoSize = m_activeCamera->orthoSize();
    m_blockMoveOnTransfer = true;
    m_transferProgress = 0.0f;
}

void CameraController::activateCamera(int index) {
    if(index < m_cameras.size()) {
        if(m_currentCamera >= 0) {
            m_cameras[m_currentCamera].ortho = m_activeCamera->orthographic();
            m_cameras[m_currentCamera].orthoSize = m_activeCamera->orthoSize();
            m_cameras[m_currentCamera].focalDistance = m_activeCamera->focalDistance();

            Transform *t = m_activeCamera->transform();
            m_cameras[m_currentCamera].position = t->position();
            m_cameras[m_currentCamera].rotation = t->rotation();
        }

        m_currentCamera = index;
        m_targetCamera = m_cameras[m_currentCamera];
        m_transferProgress = 0.0f;

        onOrthographic(m_targetCamera.ortho);
    }
}

void CameraController::resetCamera() {
    float dist = 10.0f;

    Vector3 rotation(-30.0f,-45.0f, 0.0f);

    Vector3 forward(0.0f, 0.0f, dist);
    Vector3 position(Quaternion(rotation) * forward);

    m_cameras.clear();
    m_cameras.push_back({rotation, position, dist, false});
    m_cameras.push_back({Vector3(), Vector3(0.0f, 0.0f, -10.0f), dist, true});

    Transform *t = m_activeCamera->transform();
    t->setPosition(m_cameras[0].position);
    t->setRotation(m_cameras[0].rotation);

    m_activeCamera->setFocalDistance(m_cameras[0].focalDistance);
    m_activeCamera->setOrthoSize(m_cameras[0].orthoSize);
    m_activeCamera->setOrthographic(m_cameras[0].ortho);
}

void CameraController::setFocusOn(Actor *actor, float &bottom) {
    bottom = 0.0f;
    if(actor) {
        AABBox bb(Vector3(FLT_MAX), Vector3(-FLT_MAX));
        for(auto it : actor->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }

        float radius = 1.0f;
        bottom = radius;
        Vector3 center(actor->transform()->worldPosition());
        if(bb.isValid()) {
            radius = bb.radius * 2.0f;

            Vector3 min, max;
            bb.box(min, max);
            bottom = min.y;

            center = bb.center;
        }

        if(m_activeCamera->orthographic()) {
            radius /= sinf(m_activeCamera->fov() * DEG2RAD);
        }
        radius = CLAMP(radius, FLT_EPSILON, FLT_MAX);

        Transform *camera = m_activeCamera->transform();
        m_targetCamera.position = center + camera->quaternion() * Vector3(0.0f, 0.0f, radius);
        m_targetCamera.rotation = camera->rotation();
        m_targetCamera.focalDistance = radius;
        m_targetCamera.orthoSize = radius;

        m_transferProgress = 0.0f;
    }
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
            float focal = CLAMP(m_activeCamera->focalDistance() - scale, m_zoomLimit.x, m_zoomLimit.y);

            m_activeCamera->setFocalDistance(focal);

            Transform *t = m_activeCamera->transform();
            t->setPosition(t->position() - t->quaternion() * Vector3(0.0f, 0.0f, scale));
        }
    }
}

void CameraController::cameraRotate(const Vector3 &delta) {
    Transform *t = m_activeCamera->transform();
    Vector3 euler = t->rotation() - delta;

    if(!m_cameraFree || Input::isKey(Input::KEY_LEFT_ALT) || m_transferProgress < 1.0f) {
        Vector3 forward(0.0f, 0.0f, m_activeCamera->focalDistance());
        Vector3 dir = t->position() - t->quaternion() * forward;
        t->setPosition(dir + Quaternion(euler) * forward);
    }
    t->setRotation(euler);
}

void CameraController::cameraMove(const Vector3 &delta) {
    Transform *t = m_activeCamera->transform();
    t->setPosition(t->position() - delta * ((m_activeCamera->orthographic()) ? m_activeCamera->orthoSize() : m_activeCamera->focalDistance()));
}

void CameraController::restoreState(const VariantMap &state) {
    if(m_activeCamera) {
        int currentCamera = 0;

        auto it = state.find(gOrthographic);
        if(it != state.end()) {
            bool ortho = (*it).second.toBool();
            currentCamera = ortho ? 1 : 0;
            m_cameras[currentCamera].ortho = ortho;
        }

        it = state.find(gPosition);
        if(it != state.end()) {
            m_cameras[currentCamera].rotation = (*it).second.toVector3();
        }

        it = state.find(gRotation);
        if(it != state.end()) {
            m_cameras[currentCamera].position = (*it).second.toVector3();
        }

        it = state.find(gFocal);
        if(it != state.end()) {
            m_cameras[currentCamera].focalDistance = (*it).second.toFloat();
        }

        it = state.find(gOrthoSize);
        if(it != state.end()) {
            m_cameras[currentCamera].orthoSize = (*it).second.toFloat();
        }

        it = state.find(gGridAxis);
        if(it != state.end()) {
            m_gridAxis = static_cast<Axis>((*it).second.toInt());
        }

        it = state.find(gCameras);
        if(it != state.end()) {
            m_cameras.clear();

            for(auto &camera : (*it).second.toList()) {
                VariantList fields = camera.toList();

                auto field = fields.begin();

                CameraData data;
                data.rotation = (*field).toVector3();
                ++field;
                data.position = (*field).toVector3();
                ++field;
                data.orthoSize = (*field).toFloat();
                ++field;
                data.focalDistance = (*field).toFloat();
                ++field;
                data.ortho = (*field).toBool();

                m_cameras.push_back(data);
            }
        }

        it = state.find(gCameraIndex);
        if(it != state.end()) {
            currentCamera = (*it).second.toInt();
        }

        activateCamera(currentCamera);
    }
}

VariantMap CameraController::saveState() {
    VariantMap result;
    if(m_activeCamera) {
        result[gGridAxis] = static_cast<int>(m_gridAxis);
        result[gCameraIndex] = m_currentCamera;

        Transform *t = m_activeCamera->transform();
        m_cameras[m_currentCamera].rotation = t->rotation();
        m_cameras[m_currentCamera].position = t->position();
        m_cameras[m_currentCamera].ortho = m_activeCamera->orthographic();
        m_cameras[m_currentCamera].orthoSize = m_activeCamera->orthoSize();
        m_cameras[m_currentCamera].focalDistance = m_activeCamera->focalDistance();

        VariantList cameras;
        for(auto &it : m_cameras) {
            VariantList fields;
            fields.push_back(it.rotation);
            fields.push_back(it.position);
            fields.push_back(it.orthoSize);
            fields.push_back(it.focalDistance);
            fields.push_back(it.ortho);

            cameras.push_back(fields);
        }
        result[gCameras] = cameras;
    }
    return result;
}

void CameraController::setActiveRootObject(Object *object) {
    m_activeRootObject = object;
}

void CameraController::setZoomLimits(const Vector2 &limit) {
    m_zoomLimit = limit;
}

void CameraController::drawHelpers(Object *object, bool selected) {
    if(object == nullptr) {
        return;
    }

    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        if(!actor->isEnabled()) {
            return;
        }
        bool isRenderable = false;
        for(auto &it : actor->getChildren()) {
            Actor *childActor = dynamic_cast<Actor *>(it);
            if(childActor) {
                drawHelpers(childActor, selected);
            } else {
                Component *component = dynamic_cast<Component *>(it);
                if(component) {
                    selected ? component->drawGizmosSelected() : component->drawGizmos();

                    Renderable *renderable = dynamic_cast<Renderable *>(it);
                    if(renderable) {
                        isRenderable = true;
                    }
                }
            }
        }

        if(!isRenderable && !selected) {
            Transform *t = actor->transform();
            if(t) {
                float distance = HandleTools::distanceToPoint(Matrix4(), t->worldPosition(), Handles::s_Mouse);
                if(distance <= Handles::s_Sense) {
                    select(*actor);
                }
            }
        }
    } else {
        for(auto &it : object->getChildren()) {
            drawHelpers(it, selected);
        }
    }
}
