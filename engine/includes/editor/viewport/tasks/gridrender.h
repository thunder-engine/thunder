#ifndef GRIDRENDER_H
#define GRIDRENDER_H

#include "pipelinetask.h"
#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "camera.h"
#include "transform.h"

#include "rendertarget.h"

#include "editor/editorsettings.h"

#include "../cameracontroller.h"

namespace {
    const char *gGridColor("Viewport/Grid/Color");
}

class GridRender : public PipelineTask {
public:
    GridRender() :
            m_controller(nullptr),
            m_resultTarget(Engine::objectCreate<RenderTarget>("Grid.ResultTarget")),
            m_plane(PipelineContext::defaultPlane()),
            m_grid(nullptr),
            m_scale(1.0f) {

        Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
        if(m) {
            m_grid = m->createInstance();
        }

        m_inputs.push_back("In");
        m_inputs.push_back("depthMap");

        m_outputs.push_back(std::make_pair("Result", nullptr));

        loadSettings();
    }

    void setController(CameraController *ctrl) {
        m_controller = ctrl;
    }

    void loadSettings() {
        QColor color = EditorSettings::instance()->value(gGridColor, QColor(102, 102, 102, 102)).value<QColor>();
        m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        if(m_grid) {
            m_grid->setVector4("mainColor", &m_gridColor);
        }
    }

    int scale() const {
        return m_scale;
    }

    void setInput(int index, Texture *texture) override {
        if(texture) {
            if(texture->depthBits() > 0) {
                m_resultTarget->setDepthAttachment(texture);
            } else {
                m_resultTarget->setColorAttachment(0, texture);

                m_outputs.front().second = texture;
            }
        }
    }

private:
    void exec() override {
        Camera *camera = Camera::current();

        Transform *t = camera->transform();
        Vector3 cam = t->position();
        Vector3 pos(cam.x, 0.0f, cam.z);

        Quaternion rot;

        Vector3 planeScale(1.0f);

        enum OreintationTypes {
            XZ,
            XY,
            ZY
        };

        int orientation = XZ;

        float zoom = 0.0f;

        Vector4 gridColor(m_gridColor);

        bool ortho = camera->orthographic();
        if(ortho) {
            float height = camera->orthoSize();

            m_scale = 1;
            while(m_scale < height * 0.01f) {
                m_scale *= 10;
            }

            zoom = height / (float)m_height;
            float i;
            gridColor.w = modf(height / (float)m_scale, &i);

            planeScale = Vector3(height * camera->ratio(), height, 1.0f);

            float depth = camera->farPlane() * 0.5f;
            CameraController::Axis axis = CameraController::Axis::Z;
            if(m_controller) {
                axis = m_controller->gridAxis();
            }

            switch(axis) {
                case CameraController::Axis::X: {
                    rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                    pos = Vector3(cam.x/* + ((side == CameraController::ViewSide::VIEW_LEFT) ? depth : -depth)*/, cam.y, cam.z);
                    orientation = ZY;
                } break;
                case CameraController::Axis::Y: {
                    rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                    pos = Vector3(cam.x, cam.y/* + ((side == CameraController::ViewSide::VIEW_TOP) ? -depth : depth)*/, cam.z);
                    orientation = XZ;
                } break;
                case CameraController::Axis::Z: {
                    rot = Quaternion();
                    pos = Vector3(cam.x, cam.y, cam.z/* + ((side == CameraController::ViewSide::VIEW_FRONT) ? -depth : depth)*/);
                    orientation = XY;
                } break;
                default: break;
            }
        } else {
            Ray ray = camera->castRay(0.0f, 0.0f);

            Ray::Hit hit;
            ray.intersect(Plane(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1)), &hit, true);

            float length = MIN(hit.distance, camera->farPlane()) * 0.01f;

            m_scale = 1;
            while(m_scale < length) {
                m_scale *= 10;
            }

            zoom = m_scale * 0.01f;

            planeScale = m_scale * 1000.0f;

            pos.y = 0.0f;

            rot = Quaternion(Vector3(1, 0, 0), 90.0f);
        }

        m_grid->setTransform(Matrix4(pos, rot, planeScale));
        m_grid->setBool("ortho", &ortho);
        m_grid->setInteger("scale", &m_scale);
        m_grid->setFloat("width", &zoom);
        m_grid->setInteger("orientation", &orientation);
        m_grid->setVector4("gridColor", &gridColor);

        CommandBuffer *buffer = m_context->buffer();

        buffer->beginDebugMarker("GridRender");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(m_plane, 0, CommandBuffer::TRANSLUCENT, *m_grid);

        buffer->endDebugMarker();
    }

private:
    Vector4 m_gridColor;

    CameraController *m_controller;

    RenderTarget *m_resultTarget;

    Mesh *m_plane;

    MaterialInstance *m_grid;

    int m_scale;

};

#endif // GRIDRENDER_H
