#ifndef STATICMESHGL_H
#define STATICMESHGL_H

#include <components/staticmesh.h>
#include <engine.h>

#include "resources/ameshgl.h"
#include "apipeline.h"

class AStaticMeshGL : public StaticMesh, public IDrawObjectGL {
    A_OVERRIDE(AStaticMeshGL, StaticMesh, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    void                        draw                (APipeline &pipeline, int8_t layer);

};

#endif // STATICMESHGL_H
