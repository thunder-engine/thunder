#ifndef ADECAL_H
#define ADECAL_H

#include <list>

#include <amath.h>

#include "resources/atexturegl.h"

struct decal_source_data;

class ARender;

class Shader;
class SceneGL;

/*! \struct decal_source_data
struct decal_source_data {
    /// The decal source is enabled.
    bool                        enable;

    /// Position of decal source in world space.
    Vector3                  *pos;
    /// Direction of decal source in world space.
    Vector3                  *dir;
    /// Size of decal.
    Vector2                   size;
    /// Frustum of decal.
    Vector2                   frustum;
    /// Decal source matrix.
    Matrix4                   matrix;
    /// Decal material.
    MaterialInstance          *material;
};

class ADecal {
public:
    ADecal                          ();
    ~ADecal                         ();

    decal_source_data              *create                          (Vector3 *pos, Vector3 *dir);

    void                            destroy                         (decal_source_data *decal);

    void                            update                          (TextureGL &depth);

private:
    void                            set_shader_params               (decal_source_data *decal);

    void                            draw_box_cull                   (decal_source_data *decal);

    typedef std::list<decal_source_data *>    decal_list;
    /// Container for registered lights.
    decal_list                      mDecals;

    TextureGL                      mTexture;

    Shader                        *pDecal;

    ARender                        *pRender;
};
*/
#endif // ADECAL_H
