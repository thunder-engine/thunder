#ifndef GIZMORENDER_H
#define GIZMORENDER_H

#include "pipelinetask.h"
#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "input.h"

#include "actor.h"
#include "camera.h"
#include "transform.h"

#include "rendertarget.h"

#include "gizmos.h"

#include "../cameracontroller.h"
#include "../handles.h"

#include <float.h>

const int gNaviCubeSize(125);
const float gNaviCubeDist(3.0f);

class GizmoRender : public PipelineTask {
public:
    GizmoRender() :
            m_controller(nullptr),
            m_spriteTarget(Engine::objectCreate<RenderTarget>("Gizmo.SpriteTarget")),
            m_geometryTarget(Engine::objectCreate<RenderTarget>("Gizmo.GeometryTarget")),
            m_side(0),
            m_showCube(false),
            m_showGizmos(true) {

        Gizmos::init();

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(std::make_pair("Result", nullptr));

        Material *cubeMaterial = Engine::loadResource<Material>(".embedded/navicube.shader");
        if(cubeMaterial) {
            m_cubeMaterial = cubeMaterial->createInstance();
        }

        Material *solidMaterial = Engine::loadResource<Material>(".embedded/solid.shader");
        if(solidMaterial) {
            m_solidMaterial = solidMaterial->createInstance();
        }

        m_solidCube = Engine::loadResource<Mesh>(".embedded/navicube.fbx/Cube");
        if(m_solidCube) {
            m_solidCube->setColors(Vector4Vector(m_solidCube->vertices().size(), Vector4(0.0f, 0.46f, 0.74f, 0.25f)));
        }

        Actor *cameraActor = Engine::composeActor("Camera", "Camera");
        m_camera = cameraActor->getComponent<Camera>();
        m_camera->setFar(4.0f);
        m_camera->setNear(0.1f);
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            if(texture->depthBits() > 0) {
                m_spriteTarget->setDepthAttachment(texture);
            } else {
                m_spriteTarget->setColorAttachment(0, texture);
                m_geometryTarget->setColorAttachment(0, texture);

                m_outputs.front().second = texture;
            }
        }
    }

    void showCube(bool flag) {
        m_showCube = flag;
    }

    void showGizmos(bool flag) {
        m_showGizmos = flag;
    }

private:
    void exec() override {
        if(m_controller == nullptr ) {
            return;
        }

        CommandBuffer *buffer = m_context->buffer();
        buffer->beginDebugMarker("GizmoRender");

        Gizmos::clear();

        if(m_showGizmos) {
            m_controller->drawHandles();
        }

        Camera *c = Camera::current();
        if(CommandBuffer::isInited() && c) {
            m_context->cameraReset();

            buffer->setRenderTarget(m_spriteTarget);
            Gizmos::drawSpriteBatch(buffer);

            buffer->setRenderTarget(m_geometryTarget);
            Gizmos::drawWireBatch(buffer);
            Gizmos::drawSolidBatch(buffer);

            if(m_showCube) {
                update();

                buffer->setRenderTarget(m_spriteTarget);
                Quaternion q(c->transform()->worldQuaternion());

                Transform *t = m_camera->transform();
                t->setQuaternion(q);
                t->setPosition(q * Vector3(0.0f, 0.0f, gNaviCubeDist));

                buffer->clearRenderTarget(false);
                buffer->setViewport(m_width-gNaviCubeSize, m_height-gNaviCubeSize, gNaviCubeSize, gNaviCubeSize);
                buffer->setViewProjection(m_camera->viewMatrix(), m_camera->projectionMatrix());
                buffer->drawMesh(PipelineContext::defaultCube(), 0, CommandBuffer::UI, *m_cubeMaterial);

                if(m_side != -1) {
                    buffer->drawMesh(m_solidCube, m_side, CommandBuffer::UI, *m_solidMaterial);
                }

                buffer->setViewport(0, 0, m_width, m_height);
            }
        }

        buffer->endDebugMarker();
    }

    void update() {
        Vector4 mouse(Input::mousePosition());
        mouse.x -= m_width-gNaviCubeSize;
        mouse.y -= m_height-gNaviCubeSize;

        Ray ray = m_camera->castRay(mouse.x / float(gNaviCubeSize), mouse.y / float(gNaviCubeSize));

        const Vector3Vector &v = m_solidCube->vertices();
        const IndexVector &i = m_solidCube->indices();

        float dist = FLT_MAX;
        Ray::Hit hit;

        m_side = -1;
        for(int sub = 0; sub < m_solidCube->subMeshCount(); sub++) {
            int start = m_solidCube->indexStart(sub);
            int count = m_solidCube->indexCount(sub);

            for(int p = 0; p < count; p+=3) {
                int i1 = i[start + p];
                int i2 = i[start + p + 1];
                int i3 = i[start + p + 2];
                if(ray.intersect( v[i1], v[i2], v[i3], &hit)) {
                    if(hit.distance < dist) {
                        dist = hit.distance;
                        m_side = sub;
                    }
                }
            }
        }

        m_controller->overlapPicking(m_side != -1);

        if(m_side != -1 && Input::isMouseButtonUp(Input::MOUSE_LEFT)) {

            m_controller->setGridAxis(CameraController::Axis::Y);

            switch(m_side) {
                case 0: m_controller->doRotation(Vector3( 45.0f,-45.0f, 0.0f)); break; // Front-Bottom-Left
                case 1: m_controller->doRotation(Vector3(-45.0f,-45.0f, 0.0f)); break; // Front-Top-Left
                case 2: m_controller->doRotation(Vector3( 45.0f, 135.0f, 0.0f)); break; // Back-Bottom-Right
                case 3: m_controller->doRotation(Vector3( 45.0f, 45.0f, 0.0f)); break; // Front-Bottom-Right
                case 4: m_controller->doRotation(Vector3(-45.0f, 45.0f, 0.0f)); break; // Front-Top-Right
                case 5: m_controller->doRotation(Vector3(-45.0f, 0.0f, 0.0f)); break; // Front-Top
                case 6: m_controller->doRotation(Vector3( 45.0f,-135.0f, 0.0f)); break; // Back-Bottom-Left
                case 7: m_controller->doRotation(Vector3( 45.0f, 180.0f, 0.0f)); break; // Back-Bottom
                case 8: m_controller->doRotation(Vector3( 45.0f, 0.0f, 0.0f)); break; // Front-Bottom
                case 9: m_controller->doRotation(Vector3()); m_controller->setGridAxis(CameraController::Axis::Z); break; // Front
                case 10: m_controller->doRotation(Vector3( 0.0f, 45.0f, 0.0f)); break; // Right-Front
                case 11: m_controller->doRotation(Vector3(-45.0f, 180.0f, 0.0f)); break; // Back-Top
                case 12: m_controller->doRotation(Vector3(0.0f, 180.0f, 0.0f)); m_controller->setGridAxis(CameraController::Axis::Z); break; // Back
                case 13: m_controller->doRotation(Vector3(-45.0f,-135.0f, 0.0f)); break; // Back-Top-Left
                case 14: m_controller->doRotation(Vector3( 0.0f,-135.0f, 0.0f)); break; // Left-Back
                case 15: m_controller->doRotation(Vector3(-45.0f, 135.0f, 0.0f)); break; // Back-Top-Right
                case 16: m_controller->doRotation(Vector3( 0.0f, 135.0f, 0.0f)); break; // Right-Back
                case 17: m_controller->doRotation(Vector3( 0.0f,-45.0f, 0.0f)); break; // Left-Front
                case 18: m_controller->doRotation(Vector3(0.0f,-90.0f, 0.0f)); m_controller->setGridAxis(CameraController::Axis::X); break; // Left
                case 19: m_controller->doRotation(Vector3( 45.0f,-90.0f, 0.0f)); break; // Left-Bottom
                case 20: m_controller->doRotation(Vector3(0.0f, 90.0f, 0.0f)); m_controller->setGridAxis(CameraController::Axis::X); break; // Right
                case 21: m_controller->doRotation(Vector3(-45.0f, 90.0f, 0.0f)); break; // Right-Top
                case 22: m_controller->doRotation(Vector3(90.0f, 0.0f, 0.0f)); break; // Bottom
                case 23: m_controller->doRotation(Vector3( 45.0f, 90.0f, 0.0f)); break; // Right-Bottom
                case 24: m_controller->doRotation(Vector3(-90.0f, 0.0f, 0.0f)); break; // Top
                case 25: m_controller->doRotation(Vector3(-45.0f,-90.0f, 0.0f)); break; // Left-Top
                default: break;
            }
        }
    }

private:
    CameraController *m_controller;

    RenderTarget *m_spriteTarget;
    RenderTarget *m_geometryTarget;

    MaterialInstance *m_cubeMaterial;
    MaterialInstance *m_solidMaterial;

    Mesh *m_solidCube;

    Camera *m_camera;

    int m_side;

    bool m_showCube;
    bool m_showGizmos;
};

#endif // GIZMORENDER_H
