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

    /*! \enum ShaderType */
    enum ShaderType {
        Vertex                  = (1<<0),
        Fragment                = (1<<1),
        Geometry                = (1<<2),
        TesselationControll     = (1<<3),
        TesselationEvaluation   = (1<<4)
    };

    enum FragmentMode {
        Simple                  = (1<<8),
        Depth                   = (1<<9)
    };

    typedef unordered_map<uint16_t, uint32_t> ObjectMap;

public:
    ~AMaterialGL                ();

    void                        clear           ();

    void                        loadUserData    (const VariantMap &data);

    uint32_t                    bind            (uint8_t layer);
    void                        unbind          (uint8_t);

    uint32_t                    getProgram      (uint16_t type) const;

    uint32_t                    buildShader     (uint8_t type, const string &path = string(), const string &define = string());

    TextureMap                  textures        () const { return m_Textures; }

protected:
    void                        addPragma       (const string &key, const string &value);

    string                      parseData       (const string &data, const string &define);

    string                      loadIncludes    (const string &path, const string &define = string());

    bool                        checkShader     (uint32_t shader, const string &path, bool link = false);

private:
    ObjectMap                   m_Programs;

    typedef map<string, string> PragmaMap;

    PragmaMap                   m_Pragmas;
};

#endif // MATERIALGL_H
