#include "components/astaticmeshgl.h"

#include <atools.h>

#include "agl.h"

#include "resources/amaterialgl.h"

#include <analytics/profiler.h>
#include <components/actor.h>

AStaticMeshGL::AStaticMeshGL() :
        StaticMesh() {
}

void AStaticMeshGL::draw(APipeline &pipeline, int8_t layer) {
    Actor &a    = actor();
    AMeshGL *mesh   = dynamic_cast<AMeshGL *>(m_pMesh);
    if(mesh && layer & (RAYCAST | DEFAULT | TRANSLUCENT | SHADOWCAST)) {
        if(layer & IDrawObjectGL::RAYCAST) {
            pipeline.setColor(pipeline.idCode(a.uuid()));
        }

        uint8_t surface = 0;
        for(auto s : mesh->m_Surfaces) {
            uint8_t lod     = 0;
            Mesh::Lod *l    = &s.lods[lod];
            if(/*pipeline.boxIn(s->obb) != CameraGL::FRUSTUM_OUTSIDE*/ true) {
                AMaterialGL *material   = (surface < m_Materials.size()) ? static_cast<AMaterialGL *>(m_Materials[surface]) : nullptr;
                if(material && material->bind(pipeline, layer, AMaterialGL::Static)) {
                    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbuffer[surface][lod]);
                    // Vertex pos attribute
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, xyz));
                    // UV0 Coords attribute
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, uv0));
                    // Normal vector attribute
                    glEnableVertexAttribArray(3);
                    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, n));
                    // Tangent vector attribute
                    glEnableVertexAttribArray(4);
                    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, t));
                    // Joint indices attribute
                    glEnableVertexAttribArray(5);
                    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, index));
                    // Joint weights attribute
                    glEnableVertexAttribArray(6);
                    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)offsetof(Mesh::Vertex, weight));

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibuffer[surface][lod]);

                    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDrawElements(GL_TRIANGLES, l->indices.size(), GL_UNSIGNED_INT, 0);
                    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                    PROFILER_STAT(VERTICES, l->vertices.size());
                    PROFILER_STAT(POLYGONS, l->indices.size() / 3);
                    PROFILER_STAT(DRAWCALLS, 1);

                    glDisableVertexAttribArray(6);
                    glDisableVertexAttribArray(5);
                    glDisableVertexAttribArray(4);
                    glDisableVertexAttribArray(3);
                    glDisableVertexAttribArray(1);
                    glDisableVertexAttribArray(0);

                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                    material->unbind(layer);
                }
            }
            surface++;
        }
        pipeline.resetColor();
    }
}
