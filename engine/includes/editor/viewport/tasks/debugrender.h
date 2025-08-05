#ifndef DEBUGRENDER_H
#define DEBUGRENDER_H

#include <pipelinetask.h>
#include <pipelinecontext.h>
#include <commandbuffer.h>

#include <material.h>
#include <mesh.h>

class DebugRender : public PipelineTask {
public:
    DebugRender() :
            m_material(Engine::loadResource<Material>(".embedded/debug.shader")),
            m_mesh(PipelineContext::defaultPlane()),
            m_width(0),
            m_height(0) {

    }

    void showBuffer(const std::string &buffer) {
        m_buffers[buffer] = m_material->createInstance();
    }

    void hideBuffer(const std::string &buffer) {
        auto it = m_buffers.find(buffer);
        if(it != m_buffers.end()) {
            delete it->second;
            m_buffers.erase(it);
        }
    }

    bool isBufferVisible(const std::string &buffer) {
        auto it = m_buffers.find(buffer);
        return (it != m_buffers.end());
    }

private:
    void exec() override {
        if(!m_buffers.empty()) {
            CommandBuffer *buffer = m_context->buffer();
            buffer->beginDebugMarker("DebugRender");

            int i = 0;
            for(auto &it : m_buffers) {
                it.second->setTexture("mainTexture", m_context->textureBuffer(it.first));

                float width = 0.5f;
                float height = 0.5f;

                Matrix4 m;
                m.scale(Vector3(width, height, 1.0f));
                if(i < 4) {
                    m.mat[12] = width * 0.5f + i * width - 1.0f;
                    m.mat[13] = height * 0.5f - 1.0f;
                } else {
                    m.mat[12] = width * 0.5f + (i - 4) * width - 1.0f;
                    m.mat[13] = 1.0f - height * 0.5f;
                }
                it.second->setTransform(m);

                buffer->drawMesh(m_mesh, 0, Material::Opaque, *it.second);
                i++;
            }

            buffer->endDebugMarker();
        }
    }

    void resize(int32_t width, int32_t height) override {
        m_width = width;
        m_height = height;
    }

private:
    std::map<std::string, MaterialInstance *> m_buffers;

    Material *m_material;
    Mesh *m_mesh;

    int32_t m_width;
    int32_t m_height;

};

#endif // DEBUGRENDER_H
