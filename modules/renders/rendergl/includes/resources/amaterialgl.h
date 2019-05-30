#ifndef MATERIALGL_H
#define MATERIALGL_H

#include <unordered_map>
#include <list>

#include <resources/material.h>

#include <engine.h>

class ATextureGL;
class ICommandBuffer;

class AMaterialGL : public Material {
    A_OVERRIDE(AMaterialGL, Material, Resources)

    enum ShaderType {
        Static      = 1,
        Instanced,
        Skinned,
        Particle,
        LastVertex,

        Default     = 20,
        Simple,
        LastFragment
    };

    typedef unordered_map<uint16_t, uint32_t> ObjectMap;

public:
    ~AMaterialGL                ();

    void                        clear           ();

    void                        loadUserData    (const VariantMap &data);

    uint32_t                    bind            (uint32_t layer, uint16_t vertex);

    uint32_t                    getProgram      (uint16_t type) const;

    TextureMap                  textures        () const { return m_Textures; }

protected:
    uint32_t                    buildShader     (uint16_t type, const string &src = string());

    uint32_t                    buildProgram    (uint32_t vertex, uint32_t fragment);

    bool                        checkShader     (uint32_t shader, const string &path, bool link = false);

private:
    ObjectMap                   m_Programs;

};

#endif // MATERIALGL_H
