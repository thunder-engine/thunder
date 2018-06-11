#include "commandbuffer.h"

void ICommandBuffer::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {

}

void ICommandBuffer::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {

}

void ICommandBuffer::setRenderTarget(const TargetBuffer &target, const RenderTexture *depth) {

}

Vector4 ICommandBuffer::idToColor(uint32_t id) {
    uint8_t rgb[4];
    rgb[0]  = id;
    rgb[1]  = id >> 8;
    rgb[2]  = id >> 16;
    rgb[3]  = id >> 24;

    return Vector4((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[3] / 255.0f);
}

void ICommandBuffer::setColor(const Vector4 &color) {

}

void ICommandBuffer::setScreenProjection() {
    ICommandBuffer::setViewProjection(Matrix4(), Matrix4::ortho(0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
}

void ICommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {

}

void ICommandBuffer::setGlobalValue(const char *name, const Variant &value) {

}

void ICommandBuffer::setGlobalTexture(const char *name, const Texture *value) {

}

Matrix4 ICommandBuffer::projection() const {
    return Matrix4();
}

Matrix4 ICommandBuffer::modelView() const {
    return Matrix4();
}

const Texture *ICommandBuffer::texture(const char *name) const {
    return nullptr;
}

void ICommandBuffer::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

}
