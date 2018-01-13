#include "components/alightsourcegl.h"

#include "agl.h"

#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/acameragl.h"

#include "resources/amaterialgl.h"

#include "filters/ablurgl.h"

#define MAX_LODS 1
#define SM_RESOLUTION_DEFAULT 1024

ALightSourceGL::ALightSourceGL() {
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

    m_ShadowMap.create(GL_TEXTURE_2D, GL_RG32F, GL_RGB, GL_FLOAT);
    m_ShadowMap.resize(m_Resolution, m_Resolution);

    m_ShadowTemp.create(GL_TEXTURE_2D, GL_RG32F, GL_RGB, GL_FLOAT);
    m_ShadowTemp.resize(m_Resolution, m_Resolution);

    m_Steps     = 4;
    m_pPoints   = new float[m_Steps];

    m_pPoints[0]    = 20.0f / 64.0f;
    m_pPoints[1]    = 15.0f / 64.0f;
    m_pPoints[2]    = 6.0f / 64.0f;
    m_pPoints[3]    = 1.0f / 64.0f;
}

void ALightSourceGL::draw(APipeline &pipeline, uint8_t layer) {
    glActiveTexture(GL_TEXTURE5);
    //m_ShadowMap.bind();

    switch(m_Type) {
        case LightSource::DIRECT: {
            lightDirect(pipeline, layer);
        } break;
        case LightSource::POINT: {
            lightPoint(pipeline);
        } break;
        case LightSource::SPOT: {
            lightSpot(pipeline);
        } break;
        default: break;
    }

    glActiveTexture(GL_TEXTURE5);
    m_ShadowMap.unbind();
}

void ALightSourceGL::shadowsUpdate(APipeline &pipeline) {
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
        if(m_Type == LightSource::DIRECT) {
            proj.ortho(-size, size, -size, size, -1000.0f, 1000.0f);
        }

        Matrix4 m;
        pipeline.setPos(Vector3(0.5f), m);
        pipeline.setScl(Vector3(0.5f), m);
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

void ALightSourceGL::setShaderParams(uint32_t program) {
    int location;

    Actor &a    = actor();

    location	= glGetUniformLocation(program, "light.pos");
    if(location > -1) {
        glProgramUniform3fv(program, location, 1, a.position().v);
    }
    location	= glGetUniformLocation(program, "light.dir");
    if(location > -1) {
        glProgramUniform3fv(program, location, 1, (a.rotation() * Vector3(0.0f, 0.0f, 1.0f)).v);
    }
    location	= glGetUniformLocation(program, "light.color");
    if(location > -1) {
        glProgramUniform3fv(program, location, 1, Vector3(1.0f).v);
    }
    location	= glGetUniformLocation(program, "light.brightness");
    if(location > -1) {
        glProgramUniform1f(program, location, m_Brightness);
    }
    location	= glGetUniformLocation(program, "light.radius");
    if(location > -1) {
        glProgramUniform1f(program, location, m_Radius);
    }
    location	= glGetUniformLocation(program, "light.shadows");
    if(location > -1) {
        glProgramUniform1f(program, location, (m_Shadows) ? 1.0 : 0.0 );
    }
    if(m_Shadows) {
        location    = glGetUniformLocation(program, "light.lod");
        if(location > -1) {
            glProgramUniform3fv(program, location, 1, m_Distance.v);
        }
        location    = glGetUniformLocation(program, "light.matrix");
        if(location > -1) {
            for(uint8_t layer = 0; layer < m_LODCount; layer++) {
                glProgramUniformMatrix4fv(program, location + layer, 1, false, m_pMatrix[layer].mat);
            }
        }
    }
}

void ALightSourceGL::lightDirect(APipeline &pipeline, uint8_t layer) {
    AMaterialGL *material   = pipeline.materialDirect();
    if(material) {
        pipeline.makeOrtho();
        if(material->bind(pipeline, layer, AMaterialGL::Static)) {
            setShaderParams(material->getProgram(AMaterialGL::Static));
            pipeline.drawQuad();

            material->unbind(layer);
        }
    }
}

void ALightSourceGL::lightPoint(APipeline &pipeline) {
    AMaterialGL *material   = pipeline.materialPoint();
/*
    shader.attach();

    scene.setShaderParams(shader);
    setShaderParams(shader);

    glDisable	( GL_DEPTH_TEST );
    glDepthMask	( GL_FALSE );

    glEnable	( GL_CULL_FACE );
    glCullFace	( GL_FRONT );

//    scene.drawBoxCull(m_Position, Vector3(m_Radius));

    glDisable	( GL_CULL_FACE );

    glEnable	( GL_DEPTH_TEST );
    glDepthMask	( GL_TRUE );
*/
}

void ALightSourceGL::lightSpot(APipeline &pipeline) {
    AMaterialGL *material   = pipeline.materialSpot();
/*
    shader.attach();

    int location;
    location	= pSpot->location((char *)"screen");
    if(location > -1 && pCamera)
       glUniform4f(location, mScreen.x, mScreen.y, pCamera->n, pCamera->f);
    location	= pSpot->location((char *)"model_view");
    if(location > -1)
       glUniformMatrix4fv(location, 1, GL_FALSE, model_view_matrix.mat);
    location	= pSpot->location((char *)"position");
    if(location > -1)
       glUniform3fv(location, 1, light->eye);
    location	= pSpot->location((char *)"target");
    if(location > -1)
       glUniform3fv(location, 1, light->pos);

    glPushMatrix();
    glTranslatef(light->eye.x, light->eye.y, light->eye.z);

    glMultiTexCoord3fv( GL_TEXTURE1, light->color );    // Light color
    glMultiTexCoord4f ( GL_TEXTURE2,                    // Light additional params
                        light->radius,
                        light->brightness,
                        light->fov.v[1],                    // Spread
                        light->fov.v[2] );                  // Sharpness

    glDisable	( GL_DEPTH_TEST );
    glDepthMask	( GL_FALSE );

    glEnable	( GL_CULL_FACE );
    glCullFace	( GL_FRONT );

    GLUquadric *q	= gluNewQuadric();
    gluSphere   ( q, light->radius, 20, 20 );
    gluDeleteQuadric( q );

    glDisable	( GL_CULL_FACE );

    glEnable	( GL_DEPTH_TEST );
    glDepthMask	( GL_TRUE );

    glPopMatrix();
*/
}
