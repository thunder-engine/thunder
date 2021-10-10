#include "commandbuffer.h"

static bool s_Inited = false;

void CommandBuffer::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
     A_UNUSED(clearColor);
     A_UNUSED(color);
     A_UNUSED(clearDepth);
     A_UNUSED(depth);
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
    A_UNUSED(color);
}

void CommandBuffer::setScreenProjection() {
    setViewProjection(Matrix4(), Matrix4::ortho(-0.5f, 0.5f,-0.5f, 0.5f,-1.0f, 1.0f));
}

void CommandBuffer::resetViewProjection() {

}

void CommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    A_UNUSED(view);
    A_UNUSED(projection);
}

void CommandBuffer::setGlobalValue(const char *name, const Variant &value) {
    A_UNUSED(name);
    A_UNUSED(value);
}

void CommandBuffer::setGlobalTexture(const char *name, Texture *value) {
    A_UNUSED(name);
    A_UNUSED(value);
}

Matrix4 CommandBuffer::projection() const {
    return Matrix4();
}

Matrix4 CommandBuffer::view() const {
    return Matrix4();
}

Texture *CommandBuffer::texture(const char *name) const {
    A_UNUSED(name);
    return nullptr;
}

void CommandBuffer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

void CommandBuffer::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

void CommandBuffer::disableScissor() {

}
