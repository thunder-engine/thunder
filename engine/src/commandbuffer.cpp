#include "commandbuffer.h"

#include "components/camera.h"
#include "components/transform.h"
#include "timer.h"

static bool s_Inited = false;

/*!
    \class CommandBuffer
    \brief Represents a command buffer used in a graphics rendering pipeline.
    \inmodule Engine

    The CommandBuffer class represents a command buffer used in a graphics rendering pipeline.
    It provides methods for issuing rendering commands, setting global parameters, and managing textures.
*/

CommandBuffer::CommandBuffer() {
    uint32_t size = Texture::maxTextureSize();
    m_global.params.x = 1.0f / (float)size;
}

void CommandBuffer::begin() {
    m_global.params.y = Timer::time();
    m_global.params.z = Timer::deltaTime();
}
/*!
    Dispatches a compute \a shader with the specified workgroup dimensions.
    Parameters \a groupsX, \a groupsY and \a groupsZ alows to specify a size of workgroup in each demension.
*/
void CommandBuffer::dispatchCompute(ComputeInstance &shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    A_UNUSED(shader);
    A_UNUSED(groupsX);
    A_UNUSED(groupsY);
    A_UNUSED(groupsZ);
}
/*!
    \fn void CommandBuffer::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance)

    Draws a \a mesh with the specified \a sub mesh index with assigned material \a instance, and rendering \a layer.
*/
void CommandBuffer::drawMesh(Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance &instance) {
    A_UNUSED(mesh);
    A_UNUSED(sub);
    A_UNUSED(layer);
    A_UNUSED(instance);
}
/*!
    Sets the render \a target for subsequent rendering commands.
    Parameter \a level specifies the Mipmap level.
*/
void CommandBuffer::setRenderTarget(RenderTarget *target, uint32_t level) {
    A_UNUSED(target);
    A_UNUSED(level);
}
/*!
    Converts a 32-bit \a id to a Vector4 color.
*/
Vector4 CommandBuffer::idToColor(uint32_t id) {
    uint8_t rgb[4];
    rgb[0] = id;
    rgb[1] = id >> 8;
    rgb[2] = id >> 16;
    rgb[3] = id >> 24;

    return Vector4((float)rgb[0] / 255.0f, (float)rgb[1] / 255.0f, (float)rgb[2] / 255.0f, (float)rgb[3] / 255.0f);
}
/*!
    Returns true if the CommandBuffer is initialized; otherwise, false.
*/
bool CommandBuffer::isInited() {
    return s_Inited;
}
/*!
    \internal
*/
void CommandBuffer::setInited() {
    s_Inited = true;
}
/*!
     Sets the \a view and \a projection matrices.
*/
void CommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_global.view = view;
    m_global.projection = projection;
    m_global.cameraWorldToScreen = projection * view;
}
/*!
     Sets a global \a texture based on its \a name.
*/
void CommandBuffer::setGlobalTexture(const TString &name, Texture *texture) {
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
/*!
     Retrieves a global texture based on its \a name.
*/
Texture *CommandBuffer::texture(const TString &name) const {
    PROFILE_FUNCTION();

    for(auto &it : m_textures) {
        if(it.name == name) {
            return it.texture;
        }
    }
    return nullptr;
}
/*!
    Begins a debug marker with the specified \a name.
*/
void CommandBuffer::beginDebugMarker(const TString &name) {
    A_UNUSED(name);
}
/*!
    Ends the current debug marker.
*/
void CommandBuffer::endDebugMarker() {

}
/*!
    Sets the viewport dimensions.
    Parameters \a x and \a y represents viewport coordinates.
    \a width and \a height viewport dimensions.
*/
void CommandBuffer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_global.cameraParams.z = 1.0f / (float)width;
    m_global.cameraParams.w = 1.0f / (float)height;
}
/*!
    Sets the \a camera specific global variables.
    This function sets up view, projection and clipping planes global shader variables.
*/
void CommandBuffer::setCameraProperties(Camera *camera) {
    setViewProjection(camera->viewMatrix(), camera->projectionMatrix());

    Transform *t = camera->transform();

    m_global.cameraPosition = t->worldPosition();
    m_global.cameraParams.x = camera->nearPlane();
    m_global.cameraParams.y = camera->farPlane();
}
/*!
    Enables scissor testing with the specified parameters.
    Parameters \a x and \a y represents scissor coordinates.
    \a width and \a height scissor dimensions.
*/
void CommandBuffer::enableScissor(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}
/*!
    Disables scissor testing.
*/
void CommandBuffer::disableScissor() {

}
