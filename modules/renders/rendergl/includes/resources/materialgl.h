#ifndef MATERIALGL_H
#define MATERIALGL_H

#include <unordered_map>
#include <list>

#include <resources/material.h>

#include <engine.h>

class CommandBufferGL;

class MaterialInstanceGL : public MaterialInstance {
public:
    MaterialInstanceGL(Material *material);

    ~MaterialInstanceGL();

    bool bind(CommandBufferGL *buffer, uint32_t layer);

private:
    static void setBlendState(const Material::BlendState &state);

    static void setRasterState(const Material::RasterState &state);

    static void setDepthState(const Material::DepthState &state);

    static void setStencilState(const Material::StencilState &state);

private:
    Material::BlendState m_blendState;

    Material::DepthState m_depthState;

    Material::StencilState m_stencilState;

    uint32_t m_instanceUbo;

};

class MaterialGL : public Material {
    A_OVERRIDE(MaterialGL, Material, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

    enum ShaderType {
        VertexStatic      = 1,
        VertexSkinned,
        VertexParticle,
        VertexLast,

        FragmentDefault,
        FragmentVisibility,
        FragmentLast,

        GeometryDefault,
        GeometryLast
    };

    typedef unordered_map<uint32_t, uint32_t> ObjectMap;

public:
    void loadUserData(const VariantMap &data) override;

    uint32_t bind(uint32_t layer, uint16_t vertex);

    uint32_t getProgram(uint16_t type);

    Textures &textures() { return m_textures; }

    uint32_t uniformSize() const;

protected:
    uint32_t buildShader(uint16_t type, const string &src = string());

    uint32_t buildProgram(const vector<uint32_t> &shaders);

    bool checkShader(uint32_t shader);
    bool checkProgram(uint32_t program);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

    static void setDepthState(const DepthState &state);

    static void setStencilState(const StencilState &state);

private:
    friend class MaterialInstanceGL;

    ObjectMap m_programs;

    map<uint16_t, string> m_shaderSources;
};

#endif // MATERIALGL_H
