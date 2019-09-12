#include "commandbuffer.h"

static bool s_Inited = false;

void ICommandBuffer::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
     A_UNUSED(clearColor);
     A_UNUSED(color);
     A_UNUSED(clearDepth);
     A_UNUSED(depth);
}

void ICommandBuffer::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t layer, MaterialInstance *material) {
    A_UNUSED(model);
    A_UNUSED(mesh);
    A_UNUSED(layer);
    A_UNUSED(material);
}

void ICommandBuffer::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t layer, MaterialInstance *material, bool particle) {
    A_UNUSED(models);
    A_UNUSED(count);
    A_UNUSED(mesh);
    A_UNUSED(layer);
    A_UNUSED(material);
    A_UNUSED(particle);
}

void ICommandBuffer::setRenderTarget(const TargetBuffer &target, RenderTexture *depth) {
    A_UNUSED(target);
    A_UNUSED(depth);
}

void ICommandBuffer::setRenderTarget(uint32_t target) {
    A_UNUSED(target);
}

Vector4 ICommandBuffer::idToColor(uint32_t id) {
    uint8_t rgb[4];
    rgb[0]  = id;
    rgb[1]  = id >> 8;
    rgb[2]  = id >> 16;
    rgb[3]  = id >> 24;

    return Vector4((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[3] / 255.0f);
}

bool ICommandBuffer::isInited() {
    return s_Inited;
}

void ICommandBuffer::setInited() {
    s_Inited = true;
}

void ICommandBuffer::setColor(const Vector4 &color) {
    A_UNUSED(color);
}

void ICommandBuffer::setScreenProjection() {
    setViewProjection(Matrix4(), Matrix4::ortho(-0.5f, 0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
}

void ICommandBuffer::resetViewProjection() {

}

void ICommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    A_UNUSED(view);
    A_UNUSED(projection);
}

void ICommandBuffer::setGlobalValue(const char *name, const Variant &value) {
    A_UNUSED(name);
    A_UNUSED(value);
}

void ICommandBuffer::setGlobalTexture(const char *name, Texture *value) {
    A_UNUSED(name);
    A_UNUSED(value);
}

Matrix4 ICommandBuffer::projection() const {
    return Matrix4();
}

Matrix4 ICommandBuffer::modelView() const {
    return Matrix4();
}

Texture *ICommandBuffer::texture(const char *name) const {
    A_UNUSED(name);
    return nullptr;
}

void ICommandBuffer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

void ICommandBuffer::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

void ICommandBuffer::disableScissor() {

}
