#include "resources/ameshgl.h"

#include "agl.h"

AMeshGL::AMeshGL() :
        Mesh() {

    mVBO    = false;
}

AMeshGL::~AMeshGL() {
    clear();
}

void AMeshGL::loadUserData(const AVariantMap &data) {
    Mesh::loadUserData(data);

    clear();

    bool vbo = true;
    for(uint32_t s = 0; s < m_Surfaces.size(); s++) {
        uint8_t lods    = m_Surfaces[s].lods.size();
        uint32_t *vb    = new uint32_t[lods];
        uint32_t *ib    = new uint32_t[lods];
        glGenBuffers(lods, vb);
        glGenBuffers(lods, ib);
        for(uint32_t l = 0; l < lods; l++) {
            Lod *lod    = &m_Surfaces[s].lods[l];
            if(vb[l]) {
                glBindBuffer(GL_ARRAY_BUFFER, vb[l]);
                glBufferData(GL_ARRAY_BUFFER, lod->vertices.size() * sizeof(Vertex), &lod->vertices[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            } else {
                vbo = false;
            }

            if(ib[l]) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib[l]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, lod->indices.size() * sizeof(int), &lod->indices[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            } else {
                vbo = false;
            }
        }
        vbuffer.push_back(vb);
        ibuffer.push_back(ib);
    }
    mVBO    = vbo;
}

void AMeshGL::clear() {
    if(vbuffer.empty() && ibuffer.empty()) {
        return;
    }
    for(uint32_t s = 0; s < m_Surfaces.size(); s++) {
        uint8_t lods    = m_Surfaces[s].lods.size();

        uint32_t *vb    = vbuffer[s];
        glDeleteBuffers(lods, vb);
        delete []vb;

        uint32_t *ib    = ibuffer[s];
        glDeleteBuffers(lods, ib);
        delete []vb;
    }
    vbuffer.clear();
    ibuffer.clear();
}

/*
void MeshSystemGL::attach(mesh_data *p_ready_mesh, joint_data *p_ready_array, mesh_data *p_attach_mesh, joint_data *p_attach_array, uint8_t proxy) {

    joint_data *pProxy = &p_ready_array[0];
    // Get proxy
    int j;
    for(j = 0; j < p_ready_mesh->jCount; j++) {
        if(p_ready_array[j].proxy == proxy) {
            pProxy  = &p_ready_array[j];
            break;
        }
    }
    // Attach object
    for(j = 0; j < p_attach_mesh->jCount; j++) {
        if(p_attach_array[j].iparent == -1) {
            p_attach_array[j].parent	= pProxy;
            break;
        }
    }

}

void MeshSystemGL::detach() {

}

void MeshSystemGL::cpu_calculation(mesh_instance_data *instance, joint_data *array) {
    for(int surface = 0; surface < instance->pMesh->surfaces.size(); surface++) {
        surface_data *s     = instance->pMesh->surfaces[surface];
        // \todo: maybe 1 bone bug
        for(int vertex = 0; vertex < s->vCount; vertex++) {
            vertex_data *v  = &s->vertices[vertex];

            Vector4 xyz   = Vector4(v->xyz.x, v->xyz.y, v->xyz.z, 1.0f);

            v->txyz         = Vector4();
            v->tn           = Vector3();

            for(int k = 0; k < v->wCount; k++) {
                int index   = (int)v->index[k];
                v->txyz    += array[index].transform * xyz * v->weight[k];
                v->tn      += array[index].rotation * v->n * v->weight[k];
            }
            
            glBindBuffer(GL_ARRAY_BUFFER, s->vbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, s->vCount * sizeof(vertex_data), s->vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

}
    if(pMesh->mesh_type) {
        pMesh->vCount       = pMesh->tpl->animations.size();
        if(pMesh->vCount) {
            pMesh->aAnim    = new animation_data[pMesh->vCount];

            Vector3 aabb[2];
            for(unsigned int i = 0; i < pMesh->tpl->animations.size(); i++) {
                tpl_data *pTemplate                 = pMesh->tpl->animations[i];
                tpl_animation_set_data *pAnimation  = (tpl_animation_set_data *)pTemplate->data[0];
                pLog->set_record(ALog::LOG_ERROR, pTemplate->name);
                load_animation(pAnimation->path, pTemplate->name, pMesh, i, pAnimation->type, aabb);
            }
            pMesh->aabb = AABox(aabb[0], aabb[1].x - aabb[0].x, aabb[1].y - aabb[0].y, aabb[1].z - aabb[0].z);
        }
    }
*/
