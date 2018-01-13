#include "adecal.h"

#include "agl.h"
#include "adeferredshading.h"
/*
ADecal::ADecal() {
    //pRender       = ADeferredShading::instance()->get_render();

    pDecal        = new Shader;
    pDecal->load(pRender->file(), pRender->log(), "shaders/glsl/decal.xml");

    /// \todo: Return texture
    //mTexture.load(pRender->file(), pRender->log(), "textures/icons/trigger.png");
}

ADecal::~ADecal() {
}

decal_source_data *ADecal::create(Vector3 *pos, Vector3 *dir) {
    decal_source_data *d    = new decal_source_data;

    d->enable               = true;
    d->pos                  = pos;
    d->dir                  = dir;
    d->size                 = Vector2(1.0f);
    d->frustum              = Vector2(0.01, 1.0);
    d->material             = 0;

    mDecals.push_back(d);

    return d;
}

void ADecal::destroy(decal_source_data *decal) {
}

void ADecal::update(TextureGL &depth) {
//
//  glDepthMask	( GL_FALSE );
//
//  glActiveTextureARB  ( GL_TEXTURE0_ARB);
//  depth.bind();
//
//  glActiveTextureARB  ( GL_TEXTURE1_ARB);
//  mTexture.bind();
//  /// \todo: Migrate to scene
//
//  decal_list::iterator it     = mDecals.begin();
//  while(it != mDecals.end()) {
//      decal_source_data *d    = *it;
//      if(d->enable) {
//          Vector3 up ( 0.0, 1.0, 0.0);
//
//          Matrix4 *model        = pCamera->get_model();
//
//          Matrix4 t;
//          t.translate(*d->pos);
//          Matrix4 r;
//          r.direction(*d->dir, up);
//
//          *model  = t * r;
//
//          Vector3 target        = *d->pos + *d->dir;
//
//          Matrix4 mView;
//          mView.identity();
//          mView.look_at(*d->pos, target, up);
//
//          Matrix4 mModelView    = mView * (*model);
//
//          d->matrix.identity();
//
//          t.translate (0.5, 0.5, -d->frustum.x);
//          d->matrix      *= t;
//          t.scale     (1.0 / d->size.x, 1.0 / d->size.y, -1.0 / (d->frustum.y - (2 * d->frustum.x)) );
//          d->matrix      *= t;
//
//          d->matrix       = d->matrix * mModelView;
//
//          if(pDecal)
//              pDecal->attach();
//
//          pCamera->set_shader_params(pDecal);
//          set_shader_params(d);
//
//          glEnable	( GL_CULL_FACE );
//          glCullFace	( GL_FRONT );
//
//          draw_box_cull(d);
//
//          glDisable	( GL_CULL_FACE );
//
//          if(pDecal)
//              pDecal->detach();
//
//          model->identity();
//      }
//      it++;
//  }
//
//  glActiveTextureARB  ( GL_TEXTURE1_ARB);
//  mTexture.unbind();
//
//  glActiveTextureARB  ( GL_TEXTURE0_ARB);
//  depth.unbind();
//
//  glDepthMask	( GL_TRUE );
//
}

void ADecal::set_shader_params(decal_source_data *decal) {
    int location;

    location    = pDecal->location((char *)"matrix");
    if(location > -1) {
        glUniformMatrix4fv(location, 1, false, decal->matrix);
    }
}

void ADecal::draw_box_cull(decal_source_data *decal) {
    Vector3	size    = Vector3(decal->size.x,
                                    decal->frustum.y - (2 * decal->frustum.x),
                                    decal->size.y);
    Vector3	begin   =-Vector3(size.x * 0.5f,
                                   -decal->frustum.x,
                                    size.z * 0.5f);
    float   	x2 = begin.x + size.x;
    float   	y2 = begin.y + size.y;
    float   	z2 = begin.z + size.z;

    float vVertices[24][3] = {
        // front face
        { begin.x, begin.y, z2 },
        { x2, begin.y, z2 },
        { x2, y2, z2 },
        { begin.x, y2, z2 },
        // back face
        { x2, begin.y, begin.z },
        { begin.x, begin.y, begin.z },
        { begin.x, y2, begin.z },
        { x2, y2, begin.z },
        // left face
        { begin.x, begin.y, begin.z },
        { begin.x, begin.y, z2 },
        { begin.x, y2, z2 },
        { begin.x, y2, begin.z },
        // right face
        { x2, begin.y, z2 },
        { x2, begin.y, begin.z },
        { x2, y2, begin.z },
        { x2, y2, z2 },
        // top face
        { begin.x, y2, z2 },
        { x2, y2, z2 },
        { x2, y2, begin.z },
        { begin.x, y2, begin.z },
        // bottom face
        { x2, begin.y, z2 },
        { begin.x, begin.y, z2 },
        { begin.x, begin.y, begin.z },
        { x2, begin.y, begin.z }
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vVertices);

//    AStatistic::instance().addVerts(24);
//    AStatistic::instance().addPolys(12);
//    AStatistic::instance().addCall();

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_VERTEX_ARRAY);
}
*/
