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

#include "components/alightsourcegl.h"
#include "components/acameragl.h"
#include "components/aspritegl.h"

#include "analytics/profiler.h"

APipeline::APipeline(Engine *engine) :
        m_pDirect(nullptr),
        m_pPoint(nullptr),
        m_pSpot(nullptr),
        m_pFont(nullptr) {
    m_Screen    = Vector2(64, 64);

    m_pEngine   = engine;

    m_World     = Vector3();

    m_Color     = Vector4(1.0f);

    m_pController   = NULL;

    m_Coords.push_back(Vector2( 0.0f, 0.0f ));
    m_Coords.push_back(Vector2( 1.0f, 0.0f ));
    m_Coords.push_back(Vector2( 1.0f, 1.0f ));
    m_Coords.push_back(Vector2( 0.0f, 1.0f ));

    m_Vertices.push_back(Vector4( 0.0f, 0.0f, 0.0f, 0.0f));
    m_Vertices.push_back(Vector4( 1.0f, 0.0f, 0.0f, 0.0f));
    m_Vertices.push_back(Vector4( 1.0f, 1.0f, 0.0f, 0.0f));
    m_Vertices.push_back(Vector4( 0.0f, 1.0f, 0.0f, 0.0f));

    m_pSprite   = static_cast<ASpriteGL *>(Engine::objectCreate<Sprite>());
    m_pSprite->setMaterial(Engine::loadResource<AMaterialGL>(".embedded/DefaultSprite.mtl"));

    m_pDirect   = Engine::loadResource<AMaterialGL>(".embedded/VSM.mtl");
    initLightMaterial(m_pDirect);

    //loadLightMaterial(m_Point, "shaders/PointLight.frag");
    //loadLightMaterial(m_Spot, "shaders/SpotLight.frag");

    m_pBlur     = new ABlurGL(this);
    m_pAO       = new AAmbientOcclusionGL();

    //m_PostEffects.push_back(new AAntiAliasingGL());
    //m_PostEffects.push_back(new ABloomGL());

    m_Depth.create(GL_TEXTURE_2D, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_FLOAT);

    glGenFramebuffers(1, &m_DepthBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthBuffer);

#ifdef GL_ES_VERSION_2_0
    uint32_t target = GL_DRAW_FRAMEBUFFER_APPLE;
#else
    uint32_t target = GL_DRAW_FRAMEBUFFER;
#endif
    glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0);

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

    Profiler::statReset(VERTICES);
    Profiler::statReset(POLYGONS);
    Profiler::statReset(DRAWCALLS);

    // Light prepass
    updateShadows(scene);

    analizeScene(scene, controller);

    glClearColor(c.x, c.y, c.z, c.w);
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

void APipeline::cameraSet(ACameraGL &camera) {
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
    if(location > -1) {
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

/*!
    Resizing current texture buffers.
    @param[in]  width       New screen width.
    @param[in]  height      New screen height.
*/
void APipeline::resize(int32_t width, int32_t height) {
    m_Screen    = Vector2(width, height);
    m_pAO->resize(width, height);

    for(auto it : m_PostEffects) {
        it->resize(width, height);
    }
}

void APipeline::clearScreen(const ATextureGL &target) {
    glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
    glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer   ( GL_FRAMEBUFFER, 0 );
}

void APipeline::drawQuad() {
    // Vertex pos attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, &m_Vertices[0]);
    // UV Coords attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, &m_Coords[0]);

    PROFILER_STAT(VERTICES,     4);
    PROFILER_STAT(POLYGONS,     2);
    PROFILER_STAT(DRAWCALLS,    1);
#ifdef GL_ES_VERSION_2_0
    uint32_t mode   = GL_QUADS_OES;
#else
    uint32_t mode   = GL_QUADS;
#endif
    glDrawArrays(mode, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void APipeline::drawTexturedQuad(const ATextureGL &texture) {
    glActiveTexture (GL_TEXTURE0);
    texture.bind();

    drawQuad();

    glActiveTexture (GL_TEXTURE0);
    texture.unbind();
}

void APipeline::drawScreen(const ATextureGL &source, const ATextureGL &target) {
    glBindFramebuffer       ( GL_FRAMEBUFFER, m_ScreenBuffer );
    glFramebufferTexture2D  ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.id(), 0 );

    drawTexturedQuad(source);

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
                setTransform(actor->transform());
            }
            drawComponents(*child, layer);
            if(actor) {
                resetTransform();
                resetColor();
            }
        }
    }
}

void APipeline::updateShadows(AObject &object) {
    for(auto &it : object.getChildren()) {
        ALightSourceGL *light = dynamic_cast<ALightSourceGL *>(it);
        if(light) {
            light->shadowsUpdate(*this);
        } else {
            updateShadows(*it);
        }
    }
}

void APipeline::updateLights(AObject &object, uint8_t layer) {
    for(auto &it : object.getChildren()) {
        ALightSourceGL *light = dynamic_cast<ALightSourceGL *>(it);
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

void APipeline::setTransform(const Matrix4 &mat) {
    m_MatrixStack.push(m_Model);

    loadMatrix(MATRIX_MODEL, m_Model * mat);
}

void APipeline::resetTransform() {
    loadMatrix(MATRIX_MODEL, m_MatrixStack.top());
    m_MatrixStack.pop();
}

void APipeline::setPos(const Vector3 &pos, Matrix4 &m) {
    Matrix4 t;
    t.translate(pos);
    m  *= t;
}

void APipeline::setScl(const Vector3 &scl, Matrix4 &m) {
    Matrix4 s;
    s.scale(scl);
    m  *= s;
}

void APipeline::makeOrtho() {
    m_Projection    = Matrix4();
    m_Projection.ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    m_View          = Matrix4();

    m_ModelView     = modelView();
}

Scene *APipeline::scene() {
    return m_pScene;
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
    if(material && material->bind(*this, IDrawObjectGL::DEFAULT, AMaterialGL::Static)) {
        uint32_t program    = material->getProgram(AMaterialGL::Static);

        int location;
        location	= glGetUniformLocation(program, "layer0");
        if(location > -1) {
            glUniform1i(location, 0);
        }
        location	= glGetUniformLocation(program, "layer1");
        if(location > -1) {
            glUniform1i(location, 1);
        }
        location	= glGetUniformLocation(program, "layer2");
        if(location > -1) {
            glUniform1i(location, 2);
        }
        location	= glGetUniformLocation(program, "layer3");
        if(location > -1) {
            glUniform1i(location, 3);
        }
        location	= glGetUniformLocation(program, "depthMap");
        if(location > -1) {
            glUniform1i(location, 4);
        }
        location	= glGetUniformLocation(program, "shadowMap");
        if(location > -1) {
            glUniform1i(location, 5);
        }

        material->unbind(IDrawObjectGL::DEFAULT);
    }
}

void APipeline::analizeScene(AObject &object, IController *controller) {
    // Retrive object id
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    cameraReset();

    drawComponents(object, IDrawObjectGL::RAYCAST);

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

