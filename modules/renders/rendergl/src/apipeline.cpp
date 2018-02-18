#include "apipeline.h"

#include <aobject.h>

#include "controller.h"

#include "filters/ablurgl.h"

#include "postprocess/aambientocclusiongl.h"
#include "postprocess/aantialiasinggl.h"
#include "postprocess/abloomgl.h"

#include <components/actor.h>
#include <components/scene.h>
#include <components/component.h>

#include "components/adirectlightgl.h"
#include "components/acameragl.h"
#include "components/aspritegl.h"

#include "analytics/profiler.h"

#include "resources/ameshgl.h"

#define OVERRIDE "texture0"

APipeline::APipeline(Engine *engine) :
        m_pScene(nullptr) {
    m_Screen    = Vector2(64, 64);

    m_pEngine   = engine;

    m_World     = Vector3();

    m_Color     = Vector4(1.0f);

    m_pController   = nullptr;

    m_pPlane    = Engine::loadResource<AMeshGL>(".embedded/plane.fbx");
    m_pCube     = Engine::loadResource<AMeshGL>(".embedded/cube.fbx");

    m_pMesh     = Engine::loadResource<AMaterialGL>(".embedded/DefaultMesh.mtl");
    m_pMeshInstance = m_pMesh->createInstance();

    m_pSprite   = Engine::loadResource<AMaterialGL>(".embedded/DefaultSprite.mtl");
    m_pSpriteInstance   = m_pSprite->createInstance();

    m_pBlur     = new ABlurGL(this);
    m_pAO       = new AAmbientOcclusionGL();

    //m_PostEffects.push_back(new AAntiAliasingGL());
    //m_PostEffects.push_back(new ABloomGL());

    m_Depth.create(GL_TEXTURE_2D, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_FLOAT);

    m_DepthBuffer   = 0;
    glGenFramebuffers(1, &m_DepthBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthBuffer);

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
    uint32_t target = GL_DRAW_FRAMEBUFFER_APPLE;
#else
    uint32_t target = GL_DRAW_FRAMEBUFFER;
#endif
    glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0);

    m_ScreenBuffer  = 0;
    glGenFramebuffers(1, &m_ScreenBuffer);
}

APipeline::~APipeline() {
    glDeleteFramebuffers(1, &m_ScreenBuffer);
    glDeleteFramebuffers(1, &m_DepthBuffer);
}

void APipeline::draw(Scene &scene, uint32_t) {
    m_pScene    = &scene;

    IController *controller = m_pEngine->controller();
    if(m_pController) {
        controller = m_pController;
    }
    Vector4 c;
    ACameraGL *camera   = activeCamera();
    if(camera) {
        Vector2 size  = camera->screen();
        if(m_Screen != size) {
            resize(size.x, size.y);
        }
        c   = camera->color();
    }

    Profiler::statReset(POLYGONS);
    Profiler::statReset(DRAWCALLS);

    // Light prepass
    updateShadows(scene);

    analizeScene(scene, controller);

    glClearColor(c.x, c.y, c.z, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //glDepthFunc(GL_EQUAL);

    cameraReset();
}

ATextureGL *APipeline::postProcess(ATextureGL &source) {
    ATextureGL *result  = &source;
    for(auto it : m_PostEffects) {
        result  = it->draw(*result, *this);
    }
    return result;
}

void APipeline::cameraReset() {
    ACameraGL *camera   = activeCamera();
    if(camera) {
        cameraSet(*camera);
    }
}

ACameraGL *APipeline::activeCamera() {
    if(m_pController) {
        return dynamic_cast<ACameraGL *>(m_pController->activeCamera());
    }
    return dynamic_cast<ACameraGL *>(m_pEngine->controller()->activeCamera());
}

void APipeline::cameraSet(const Camera &camera) {
    glViewport(0, 0, m_Screen.x, m_Screen.y);

    Matrix4 view;
    Matrix4 model;
    Matrix4 projection;

    camera.matrices(view, projection);

    loadMatrix(MATRIX_PROJECTION, projection);
    loadMatrix(MATRIX_MODEL, model);
    loadMatrix(MATRIX_VIEW, view);
}

void APipeline::setShaderParams(uint32_t program) {
    int location;
    location	= glGetUniformLocation(program, "light.ambient");
    if(location > -1 && m_pScene) {
        glUniform1f(location, m_pScene->ambient());
    }
    location    = glGetUniformLocation(program, "transform.color");
    if(location > -1) {
        glUniform4fv(location, 1, m_Color.v);
    }
    location	= glGetUniformLocation(program, "transform.view");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_View.mat);
    }
    location	= glGetUniformLocation(program, "transform.model");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_Model.mat);
    }
    location	= glGetUniformLocation(program, "transform.projection");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_Projection.mat);
    }
    location	= glGetUniformLocation(program, "transform.mv");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_ModelView.mat);
    }
    location	= glGetUniformLocation(program, "transform.mvpi");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, m_ModelViewProjectionInversed.mat);
    }
    location	= glGetUniformLocation(program, "camera.screen");
    if(location > -1) {
        glUniform2f(location, 1.0f / m_Screen.x, 1.0f / m_Screen.y);
    }
    ACameraGL *camera   = activeCamera();
    if(camera) {
        camera->setShaderParams(program);
    }
}

void APipeline::resize(int32_t width, int32_t height) {
    m_Screen    = Vector2(width, height);
    m_pAO->resize(width, height);

    for(auto it : m_PostEffects) {
        it->resize(width, height);
    }
}

void APipeline::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    material    = (material) ? material : m_pMeshInstance;
    if(mesh && material) {
        loadMatrix(MATRIX_MODEL, model);

        AMaterialGL *mat    = dynamic_cast<AMaterialGL *>(material->material());
        if(mat->bind(*this, layer, Material::Static)) {
            auto it = m_vaoMap.find(mesh);
            if(mesh->isModified() || it == m_vaoMap.end()) {
                uploadMesh(mesh);
                mesh->setModified(false);
                it  = m_vaoMap.find(mesh);
            }

            uint32_t lod    = 0;
            uint8_t flags   = mesh->flags();
            uint32_t id     = (*it).second[surface][lod];

            glBindVertexArray(id);
            // vertices
            glEnableVertexAttribArray(0);
            if(flags & Mesh::ATTRIBUTE_UV0) {
                glEnableVertexAttribArray(1);
            }
            //// uv1
            //glEnableVertexAttribArray(2);
            //// uv2
            //glEnableVertexAttribArray(3);
            //// uv3
            //glEnableVertexAttribArray(4);
            if(flags & Mesh::ATTRIBUTE_NORMALS) {
                glEnableVertexAttribArray(5);
            }
            if(flags & Mesh::ATTRIBUTE_TANGENTS) {
                glEnableVertexAttribArray(6);
            }
            //// colors
            //glEnableVertexAttribArray(5);
            //// indices
            //glEnableVertexAttribArray(8);
            //// weights
            //glEnableVertexAttribArray(9);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t count  = mesh->vertexCount(surface, lod);
                glDrawArrays((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, count);
            } else {
                uint32_t count  = mesh->indexCount(surface, lod);
                glDrawElements((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               count, GL_UNSIGNED_INT, 0);

                PROFILER_STAT(POLYGONS, count / 3);
            }

            PROFILER_STAT(DRAWCALLS, 1);

            // vertices
            glDisableVertexAttribArray(0);
            // uv0
            glDisableVertexAttribArray(1);
            // uv1
            glDisableVertexAttribArray(2);
            // uv2
            glDisableVertexAttribArray(3);
            // uv3
            glDisableVertexAttribArray(4);
            // normals
            glDisableVertexAttribArray(5);
            // tangents
            glDisableVertexAttribArray(6);
            // colors
            glDisableVertexAttribArray(7);
            // indices
            glDisableVertexAttribArray(8);
            // weights
            glDisableVertexAttribArray(9);

            glBindVertexArray(0);

            mat->unbind(layer);
        }
    }
}

void APipeline::drawQuad(const Matrix4 &model, uint8_t layer, MaterialInstance *material, const Texture *texture) {
    Material *m = (material) ? material->material() : m_pSprite;
    const Texture *t    = m->texture(OVERRIDE);
    if(texture) {
        m->overrideTexture(OVERRIDE, texture);
    }
    drawMesh(model, m_pPlane, 0, layer, (material) ? material : m_pSpriteInstance);
    m->overrideTexture(OVERRIDE, t);
}

void APipeline::drawScreen(const ATextureGL &source, const ATextureGL &target) {
    glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
    glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );

    //drawTexturedQuad(source);

    glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
}

void APipeline::drawComponents(AObject &object, uint8_t layer) {
    for(auto &it : object.getChildren()) {
        AObject *child  = it;
        IDrawObjectGL *draw = dynamic_cast<IDrawObjectGL *>(child);
        if(draw) {
            draw->draw(*this, layer);
        } else {
            Actor *actor    = dynamic_cast<Actor *>(child);
            if(actor) {
                if(!actor->isEnable()) {
                    continue;
                }
            }
            drawComponents(*child, layer);
            if(actor) {
                resetColor();
            }
        }
    }
}

void APipeline::updateShadows(AObject &object) {
    for(auto &it : object.getChildren()) {
        ADirectLightGL *light = dynamic_cast<ADirectLightGL *>(it);
        if(light) {
            light->shadowsUpdate(*this);
        } else {
            updateShadows(*it);
        }
    }
}

void APipeline::updateLights(AObject &object, uint8_t layer) {
    for(auto &it : object.getChildren()) {
        ADirectLightGL *light = dynamic_cast<ADirectLightGL *>(it);
        if(light) {
            light->draw(*this, layer);
        } else {
            updateLights(*it, layer);
        }
    }
}

Vector4 APipeline::idCode(uint32_t id) {
    uint8_t rgb[4];
    rgb[0]  = id;
    rgb[1]  = id >> 8;
    rgb[2]  = id >> 16;
    rgb[3]  = id >> 24;

    return Vector4((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[3] / 255.0f);
}

void APipeline::makeOrtho() {
    m_Projection    = Matrix4();
    m_Projection.ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f);
    m_View          = Matrix4();

    m_ModelView     = modelView();
}

void APipeline::loadMatrix(matrix_types type, const Matrix4 &m) {
    switch(type) {
        case MATRIX_MODEL:
        case MATRIX_VIEW: {
            if(type == MATRIX_MODEL) {
                m_Model     = m;
            } else {
                m_View      = m;
            }
            m_ModelView     = modelView();
            m_ModelViewProjectionInversed   = (m_Projection * m_ModelView).inverse();
        } break;
        case MATRIX_PROJECTION: {
            m_Projection    = m;
            m_ModelViewProjectionInversed   = (m_Projection * m_ModelView).inverse();
        } break;
        default: break;
    }
}

void APipeline::initLightMaterial(AMaterialGL *material) {

}

void APipeline::analizeScene(AObject &object, IController *controller) {
    // Retrive object id
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    cameraReset();

    drawComponents(object, IRenderSystem::RAYCAST);

    Vector2 position;
    if(controller) {
        Vector2 v;
        controller->selectGeometry(position, v);
    }
    Vector3 screen    = Vector3(position.x, position.y, 0.0f);
    // Screen to world
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    screen.y    = (m_Screen.y - screen.y);

    glReadPixels((int)screen.x, (int)screen.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screen.z);
    Camera::unproject(screen, modelView(), m_Projection, viewport, m_World);
    // Get id
    uint32_t result = 0;
    if(position.x >= 0.0f && position.y >= 0.0f &&
        position.x < m_Screen.x && position.y < m_Screen.y) {

        uint8_t value[4];
        glReadPixels(position.x, (m_Screen.y - position.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, value);

        result  = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
    }
    list<uint32_t> l;
    if(result) {
        l.push_back(result);
    }
    controller->setSelectedObjects(l);
}

void APipeline::uploadMesh(Mesh *mesh) {
    if(mesh) {
        clearMesh(mesh);

        AMeshGL *m  = static_cast<AMeshGL *>(mesh);
        m->createVbo();

        AMeshGL::BufferVector m_vaoVector;
        for(uint32_t s = 0; s < mesh->surfacesCount(); s++) {
            uint32_t lods   = mesh->lodsCount(s);
            Mesh::IndexVector vao   = Mesh::IndexVector(lods);

            uint8_t flags   = mesh->flags();

            glGenVertexArrays(lods, &vao[0]);
            for(uint32_t lod = 0; lod < lods; lod++) {
                glBindVertexArray(vao[lod]);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->m_triangles[s][lod]);
                // vertices
                glBindBuffer(GL_ARRAY_BUFFER, m->m_vertices[s][lod]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

                if(flags & Mesh::ATTRIBUTE_UV0) {
                    glBindBuffer(GL_ARRAY_BUFFER, m->m_uv0[s][lod]);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
                }
                //// uv1
                //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
                //// uv2
                //glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
                //// uv3
                //glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);
                if(flags & Mesh::ATTRIBUTE_NORMALS) {
                    glBindBuffer(GL_ARRAY_BUFFER, m->m_normals[s][lod]);
                    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, 0);
                }
                if(flags & Mesh::ATTRIBUTE_TANGENTS) {
                    glBindBuffer(GL_ARRAY_BUFFER, m->m_tangents[s][lod]);
                    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 0, 0);
                }
                //// colors
                //glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 0, 0);
                //// indices
                //glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 0, 0);
                //// weights
                //glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 0, 0);

                glBindVertexArray(0);
            }
            m_vaoVector.push_back(vao);
        }
        m_vaoMap[mesh]  = m_vaoVector;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void APipeline::clearMesh(const Mesh *mesh) {
    if(mesh) {
        auto it = m_vaoMap.find(mesh);
        if(it != m_vaoMap.end()) {
            for(uint32_t s = 0; s < (*it).second.size(); s++) {
                glDeleteVertexArrays((*it).second[s].size(), &(*it).second[s][0]);
            }
            m_vaoMap.erase(it);
        }
    }
}

