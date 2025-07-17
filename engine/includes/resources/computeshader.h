#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

#include "resource.h"

class Texture;
class ComputeBuffer;

class ComputeInstance;

class ENGINE_EXPORT ComputeShader : public Resource {
    A_OBJECT(ComputeShader, Resource, Resources)

public:
    struct TextureItem {
        TString name;

        Texture *texture;

        int32_t binding;

        int32_t flags;
    };

    struct BufferItem {
        TString name;

        ComputeBuffer *buffer;

        int32_t binding;

        int32_t flags;
    };

    struct UniformItem {
        TString name;

        Variant value;

        size_t size;

        size_t offset;
    };

    typedef std::list<TextureItem> TextureList;
    typedef std::list<BufferItem> BufferList;
    typedef std::list<UniformItem> UniformList;

public:
    ComputeShader();
    ~ComputeShader();

    virtual ComputeInstance *createInstance();

    void loadUserData(const VariantMap &data) override;

protected:
    void initInstance(ComputeInstance *instance);

    void switchState(Resource::State state) override;
    bool isUnloadable() override;

protected:
    friend class ComputeInstance;

    TextureList m_textures;

    BufferList m_buffers;

    UniformList m_uniforms;

    uint32_t m_uniformSize;

};

class ENGINE_EXPORT ComputeInstance {
public:
    explicit ComputeInstance(ComputeShader *compute);
    virtual ~ComputeInstance();

    ComputeShader *compute() const;

    void setBool(const TString &name, const bool *value, int32_t count = 1);

    void setInteger(const TString &name, const int32_t *value, int32_t count = 1);

    void setFloat(const TString &name, const float *value, int32_t count = 1);
    void setVector2(const TString &name, const Vector2 *value, int32_t count = 1);
    void setVector3(const TString &name, const Vector3 *value, int32_t count = 1);
    void setVector4(const TString &name, const Vector4 *value, int32_t count = 1);

    void setMatrix4(const TString &name, const Matrix4 *value, int32_t count = 1);

    void setValue(const TString &name, const void *value);

    Texture *texture(const TString &name);
    virtual void setTexture(const TString &name, Texture *texture);

    ComputeBuffer *buffer(const TString &name);
    virtual void setBuffer(const TString &name, ComputeBuffer *buffer);

protected:
    friend class ComputeShader;

    std::map<TString, Texture *> m_textureOverride;
    std::map<TString, ComputeBuffer *> m_bufferOverride;

    ComputeShader *m_compute;

    uint8_t *m_uniformBuffer;

    bool m_uniformDirty;

};

#endif // COMPUTESHADER_H
