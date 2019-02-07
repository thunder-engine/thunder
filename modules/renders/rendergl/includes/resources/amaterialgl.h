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
        Static      = 0,
        Instanced,
        Skinned,
        Particle,

        Default     = 20,
        Simple
    };

    typedef unordered_map<uint16_t, uint32_t> ObjectMap;

public:
    ~AMaterialGL                ();

    void                        clear           ();

    void                        loadUserData    (const VariantMap &data);

    uint32_t                    bind            (uint8_t layer);
    void                        unbind          (uint8_t);

    uint32_t                    getProgram      (uint16_t type) const;

    uint32_t                    buildShader     (uint8_t type, const string &src = string());

    TextureMap                  textures        () const { return m_Textures; }

protected:
    bool                        checkShader     (uint32_t shader, const string &path, bool link = false);

private:
    ObjectMap                   m_Programs;

};

#endif // MATERIALGL_H
