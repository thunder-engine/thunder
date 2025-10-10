#ifndef MATERIALGL_H
#define MATERIALGL_H

#include <unordered_map>

#include <resources/material.h>

#include <engine.h>

class CommandBufferGL;
struct Global;

class MaterialInstanceGL : public MaterialInstance {
public:
    explicit MaterialInstanceGL(Material *material);

    ~MaterialInstanceGL();

    uint32_t drawsCount() const;

    bool bind(CommandBufferGL *buffer, uint32_t layer, uint32_t index, const Global &global);

private:
    static void setBlendState(const Material::BlendState &state);

    static void setRasterState(const Material::RasterState &state);

    static void setDepthState(const Material::DepthState &state);

    static void setStencilState(const Material::StencilState &state);

private:
    Material::BlendState m_glBlendState;

    Material::DepthState m_glDepthState;

    Material::StencilState m_glStencilState;

    uint32_t m_instanceBuffer;
    uint32_t m_globalBuffer;

};

class MaterialGL : public Material {
    A_OBJECT_OVERRIDE(MaterialGL, Material, Resources)

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
    };

    typedef std::unordered_map<uint32_t, uint32_t> ObjectMap;

public:
    void loadUserData(const VariantMap &data) override;

    uint32_t getProgram(uint16_t type, int32_t &global, int32_t &local);

    Textures &textures() { return m_textures; }

protected:
    uint32_t buildShader(uint16_t type, const TString &src = TString());

    uint32_t buildProgram(const std::vector<uint32_t> &shaders, uint16_t vertex);

    bool checkShader(uint32_t shader);
    bool checkProgram(uint32_t program);

    MaterialInstance *createInstance(SurfaceType type = SurfaceType::Static) override;

    void switchState(State state) override;

private:
    friend class MaterialInstanceGL;

    ObjectMap m_programs;
    ObjectMap m_globals;
    ObjectMap m_locals;

    std::map<uint16_t, TString> m_shaderSources;

};

#endif // MATERIALGL_H
