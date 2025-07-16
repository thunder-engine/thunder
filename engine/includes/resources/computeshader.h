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
        String name;

        Texture *texture;

        int32_t binding;

        int32_t flags;
    };

    struct BufferItem {
        String name;

        ComputeBuffer *buffer;

        int32_t binding;

        int32_t flags;
    };

    struct UniformItem {
        String name;

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

    void setBool(const String &name, const bool *value, int32_t count = 1);

    void setInteger(const String &name, const int32_t *value, int32_t count = 1);

    void setFloat(const String &name, const float *value, int32_t count = 1);
    void setVector2(const String &name, const Vector2 *value, int32_t count = 1);
    void setVector3(const String &name, const Vector3 *value, int32_t count = 1);
    void setVector4(const String &name, const Vector4 *value, int32_t count = 1);

    void setMatrix4(const String &name, const Matrix4 *value, int32_t count = 1);

    void setValue(const String &name, const void *value);

    Texture *texture(const String &name);
    virtual void setTexture(const String &name, Texture *texture);

    ComputeBuffer *buffer(const String &name);
    virtual void setBuffer(const String &name, ComputeBuffer *buffer);

protected:
    friend class ComputeShader;

    std::map<String, Texture *> m_textureOverride;
    std::map<String, ComputeBuffer *> m_bufferOverride;

    ComputeShader *m_compute;

    uint8_t *m_uniformBuffer;

    bool m_uniformDirty;

};

#endif // COMPUTESHADER_H
