#include "commandbuffer.h"

#include <cstring>

static bool s_Inited = false;

CommandBuffer::CommandBuffer() :
    m_screenProjection(false),
    m_viewportX(0),
    m_viewportY(0),
    m_viewportWidth(1),
    m_viewportHeight(1) {

}

void CommandBuffer::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
     A_UNUSED(clearColor);
     A_UNUSED(color);
     A_UNUSED(clearDepth);
     A_UNUSED(depth);
}

void CommandBuffer::dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    A_UNUSED(shader);
    A_UNUSED(groupsX);
    A_UNUSED(groupsY);
    A_UNUSED(groupsZ);
}

void CommandBuffer::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    A_UNUSED(model);
    A_UNUSED(mesh);
    A_UNUSED(sub);
    A_UNUSED(layer);
    A_UNUSED(material);
}

void CommandBuffer::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    A_UNUSED(models);
    A_UNUSED(count);
    A_UNUSED(mesh);
    A_UNUSED(sub);
    A_UNUSED(layer);
    A_UNUSED(material);
}

void CommandBuffer::setRenderTarget(RenderTarget *target, uint32_t level) {
    A_UNUSED(target);
    A_UNUSED(level);
}

Vector4 CommandBuffer::idToColor(uint32_t id) {
    uint8_t rgb[4];
    rgb[0] = id;
    rgb[1] = id >> 8;
    rgb[2] = id >> 16;
    rgb[3] = id >> 24;

    return Vector4((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[3] / 255.0f);
}

bool CommandBuffer::isInited() {
    return s_Inited;
}

void CommandBuffer::setInited() {
    s_Inited = true;
}

void CommandBuffer::setColor(const Vector4 &color) {
    m_local.color = color;
}

void CommandBuffer::setObjectId(uint32_t id) {
    m_local.objectId = idToColor(id);
}

void CommandBuffer::setMaterialId(uint32_t id) {
    m_local.materialId = idToColor(id);
}

void CommandBuffer::setScreenProjection(float x, float y, float width, float height) {
    if(!m_screenProjection) {
        setViewProjection(Matrix4(), Matrix4::ortho(x, width, y, height, 0.0f, 100.0f));
        m_screenProjection = true;
    }
}

void CommandBuffer::resetViewProjection() {
    m_global.view = m_saveView;
    m_global.projection = m_saveProjection;
    m_screenProjection = false;
}

void CommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_saveView = m_global.view;
    m_saveProjection = m_global.projection;

    m_global.view = view;
    m_global.projection = projection;
}

void CommandBuffer::setGlobalValue(const char *name, const Variant &value) {
    static const unordered_map<string, pair<size_t, size_t>> offsets = {
        {"camera.position",      make_pair(offsetof(Global, cameraPosition),      sizeof(Global::cameraPosition))},
        {"camera.target",        make_pair(offsetof(Global, cameraTarget),        sizeof(Global::cameraTarget))},
        {"camera.view",          make_pair(offsetof(Global, cameraView),          sizeof(Global::cameraView))},
        {"camera.projectionInv", make_pair(offsetof(Global, cameraProjectionInv), sizeof(Global::cameraProjectionInv))},
        {"camera.projection",    make_pair(offsetof(Global, cameraProjection),    sizeof(Global::cameraProjection))},
        {"camera.screenToWorld", make_pair(offsetof(Global, cameraScreenToWorld), sizeof(Global::cameraScreenToWorld))},
        {"camera.worldToScreen", make_pair(offsetof(Global, cameraWorldToScreen), sizeof(Global::cameraWorldToScreen))},
        {"camera.screen",        make_pair(offsetof(Global, cameraScreen),        sizeof(Global::cameraScreen))},
        {"shadow.pageSize",      make_pair(offsetof(Global, shadowPageSize),      sizeof(Global::shadowPageSize))},
    };

    auto it = offsets.find(name);
    if(it != offsets.end()) {
        void *src = value.data();
        memcpy((uint8_t *)&m_global + it->second.first, src, it->second.second);
    }
}

void CommandBuffer::setGlobalTexture(const char *name, Texture *texture) {
    PROFILE_FUNCTION();

    for(auto &it : m_textures) {
        if(it.name == name) {
            it.texture = texture;
            return;
        }
    }

    Material::TextureItem item;
    item.name = name;
    item.texture = texture;
    item.binding = -1;
    m_textures.push_back(item);
}

Matrix4 CommandBuffer::projection() const {
    return m_global.projection;
}

Matrix4 CommandBuffer::view() const {
    return m_global.view;
}

Texture *CommandBuffer::texture(const char *name) const {
    PROFILE_FUNCTION();

    for(auto &it : m_textures) {
        if(it.name == name) {
            return it.texture;
        }
    }
    return nullptr;
}

Vector2 CommandBuffer::viewport() const {
    return Vector2(m_viewportWidth, m_viewportHeight);
}

void CommandBuffer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;

    setGlobalValue("camera.screen", Vector4(width, height, 1.0f / (float)width, 1.0f / (float)height));
}

void CommandBuffer::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

void CommandBuffer::disableScissor() {

}
