#ifndef MATERIALGL_H
#define MATERIALGL_H

#include <map>
#include <list>

#include <resources/material.h>

#include <engine.h>

class ATextureGL;
class CommandBufferGL;

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

public:
    AMaterialGL                 ();
    ~AMaterialGL                ();

    void                        loadUserData    (const AVariantMap &data);

    uint32_t                    bind            (MaterialInstance *instance, uint8_t layer, uint16_t type);
    void                        unbind          (uint8_t layer);

    uint32_t                    getProgram      (uint16_t type) const;

protected:
    void                        clear           ();

    uint32_t                    buildShader     (uint8_t type, const string &src, const string &path = string());

    uint32_t                    buildProgram    (uint32_t fragment, const string &define);

    bool                        checkShader     (uint32_t shader, const string &path, bool link = false);

    void                        addPragma       (const string &key, const string &value);

    string                      parseData       (const string &data, const string &define);

    string                      loadIncludes    (const string &path, const string &define = string());

private:
    typedef map<uint16_t, uint32_t> ProgramMap;

    ProgramMap                  m_Programs;

    typedef map<string, string> PragmaMap;

    PragmaMap                   m_Pragmas;
};

#endif // MATERIALGL_H
