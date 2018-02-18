#include "components/adirectlightgl.h"

#include "agl.h"

#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/acameragl.h"

#include "resources/amaterialgl.h"

#include "filters/ablurgl.h"

#define MAX_LODS 1
#define SM_RESOLUTION_DEFAULT 1024

ADirectLightGL::ADirectLightGL() {
    m_LODCount      = 0;
    m_pMatrix       = 0;
    m_Resolution    = SM_RESOLUTION_DEFAULT;
    m_CSM           = false;
    m_Distance      = Vector3();
    m_LOD           = Vector4(2.0f, 4.0f, 8.0f, 32.0f);
    m_Dir           = Vector3(1.0f, 0.0f, 0.0f);

    if(m_CSM) {
        m_LODCount  = MAX_LODS;
    } else {
        m_LODCount  = 1;
    }

    if(m_LODCount) {
        m_pMatrix  = new Matrix4[m_LODCount];
    }
    // LOD distance
    for(int i = 0; i < 3; i++) {
        m_Distance[i]   = 5;
    }

#if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
    uint32_t format = GL_RG32F_EXT;
#else
    uint32_t format = GL_RG32F;
#endif

    m_ShadowMap.create(GL_TEXTURE_2D, format, GL_RGB, GL_FLOAT);
    m_ShadowMap.resize(m_Resolution, m_Resolution);

    m_ShadowTemp.create(GL_TEXTURE_2D, format, GL_RGB, GL_FLOAT);
    m_ShadowTemp.resize(m_Resolution, m_Resolution);

    m_Steps     = 4;
    m_pPoints   = new float[m_Steps];

    m_pPoints[0]    = 20.0f / 64.0f;
    m_pPoints[1]    = 15.0f / 64.0f;
    m_pPoints[2]    = 6.0f / 64.0f;
    m_pPoints[3]    = 1.0f / 64.0f;

    m_pMaterial     = Engine::loadResource<AMaterialGL>(".embedded/VSM.mtl");
    m_pMaterialInstance = m_pMaterial->createInstance();
}

void ADirectLightGL::draw(APipeline &pipeline, uint8_t layer) {
    glActiveTexture(GL_TEXTURE5);
    m_ShadowMap.bind();

    if(m_pMaterial) {
        if(m_pMaterial->bind(pipeline, IRenderSystem::DEFAULT, AMaterialGL::Static)) {
            uint32_t program    = m_pMaterial->getProgram(AMaterialGL::Static);

            int location;
            location    = glGetUniformLocation(program, "layer0");
            if(location > -1) {
                glUniform1i(location, 0);
            }
            location    = glGetUniformLocation(program, "layer1");
            if(location > -1) {
                glUniform1i(location, 1);
            }
            location    = glGetUniformLocation(program, "layer2");
            if(location > -1) {
                glUniform1i(location, 2);
            }
            location    = glGetUniformLocation(program, "layer3");
            if(location > -1) {
                glUniform1i(location, 3);
            }
            location    = glGetUniformLocation(program, "depthMap");
            if(location > -1) {
                glUniform1i(location, 4);
            }
            location    = glGetUniformLocation(program, "shadowMap");
            if(location > -1) {
                glUniform1i(location, 5);
            }
            setShaderParams(m_pMaterial->getProgram(AMaterialGL::Static));
            m_pMaterial->unbind(IRenderSystem::DEFAULT);
        }

        pipeline.makeOrtho();
        pipeline.drawMesh(Matrix4(), pipeline.meshPlane(), 0, layer, m_pMaterialInstance);
    }

    glActiveTexture(GL_TEXTURE5);
    m_ShadowMap.unbind();
}

void ADirectLightGL::shadowsUpdate(APipeline &pipeline) {
    if(isEnable() && m_Shadows) {
        glBindFramebuffer(GL_FRAMEBUFFER, pipeline.depthBuffer());
        pipeline.depthTexture().resize(m_Resolution, m_Resolution);
        glViewport(0, 0, m_Resolution, m_Resolution);

        Matrix4 model;
        model.translate(pipeline.activeCamera()->actor().position());
        pipeline.loadMatrix(APipeline::MATRIX_MODEL, model);

        Matrix4 view  = Matrix4(actor().rotation().toMatrix()).inverse();
        pipeline.loadMatrix(APipeline::MATRIX_VIEW, view);
        // Draw in the depth buffer from position of the light source
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ShadowMap.id(), 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        float size  = 10.0f;

        Matrix4 proj;
        proj.ortho(-size, size, -size, size, -1000.0f, 1000.0f);

        Matrix4 m(Vector3(0.5f), Quaternion(), Vector3(0.5f));
        m   *= proj;
        m   *= view;
        m   *= model;

        m_pMatrix[0]    = m;

        pipeline.loadMatrix(APipeline::MATRIX_PROJECTION, proj);
        //pipeline.drawComponents(*(pipeline.scene()), IDrawObjectGL::SHADOWCAST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Blur shadow map
/*
        pipeline.cameraReset();
        pipeline.loadMatrix(APipeline::MATRIX_MODEL, Matrix4());
        ABlurGL *blur   = pipeline.filterBlur();
        if(blur) {
            Vector2 size(m_ShadowMap.width(), m_ShadowMap.height());
            blur->draw(m_ShadowMap, m_ShadowMap, m_ShadowTemp, size, m_Steps, m_pPoints);
        }
*/
    }
}

void ADirectLightGL::setShaderParams(uint32_t program) {
    int location;

    Actor &a    = actor();

    location	= glGetUniformLocation(program, "light.pos");
    if(location > -1) {
        glUniform3fv(location, 1, a.position().v);
    }
    location	= glGetUniformLocation(program, "light.dir");
    if(location > -1) {
        glUniform3fv(location, 1, (a.rotation() * Vector3(0.0f, 0.0f, 1.0f)).v);
    }
    location	= glGetUniformLocation(program, "light.color");
    if(location > -1) {
        glUniform3fv(location, 1, m_Color.v);
    }
    location	= glGetUniformLocation(program, "light.brightness");
    if(location > -1) {
        glUniform1f(location, m_Brightness);
    }
    location	= glGetUniformLocation(program, "light.shadows");
    if(location > -1) {
        glUniform1f(location, (m_Shadows) ? 1.0 : 0.0 );
    }
    if(m_Shadows) {
        location    = glGetUniformLocation(program, "light.lod");
        if(location > -1) {
            glUniform3fv(location, 1, m_Distance.v);
        }
        location    = glGetUniformLocation(program, "light.matrix");
        if(location > -1) {
            for(uint8_t layer = 0; layer < m_LODCount; layer++) {
                glUniformMatrix4fv(location + layer, 1, false, m_pMatrix[layer].mat);
            }
        }
    }
}
