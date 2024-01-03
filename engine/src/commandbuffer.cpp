#include "commandbuffer.h"

#include <cstring>

static bool s_Inited = false;

/*!
    \class CommandBuffer
    \brief Represents a command buffer used in a graphics rendering pipeline.
    \inmodule Engine

    The CommandBuffer class represents a command buffer used in a graphics rendering pipeline.
    It provides methods for issuing rendering commands, setting global parameters, and managing textures.
*/

CommandBuffer::CommandBuffer() :
    m_screenProjection(false),
    m_viewportX(0),
    m_viewportY(0),
    m_viewportWidth(1),
    m_viewportHeight(1) {

}
/*!
    Clears the render target with the specified \a color and \a depth values.
    Flag \a clearColor indicating whether to clear the color buffer.
    Flag \a clearDepth indicating whether to clear the depth buffer.
*/
void CommandBuffer::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
     A_UNUSED(clearColor);
     A_UNUSED(color);
     A_UNUSED(clearDepth);
     A_UNUSED(depth);
}
/*!
    Dispatches a compute \a shader with the specified workgroup dimensions.
    Parameters \a groupsX, \a groupsY and \a groupsZ alows to specify a size of workgroup in each demension.
*/
void CommandBuffer::dispatchCompute(ComputeInstance *shader, int32_t groupsX, int32_t groupsY, int32_t groupsZ) {
    A_UNUSED(shader);
    A_UNUSED(groupsX);
    A_UNUSED(groupsY);
    A_UNUSED(groupsZ);
}
/*!
    Draws a \a mesh with the specified \a sub mesh index in the \a transform location with assigned \a material, and rendering \a layer.
*/
void CommandBuffer::drawMesh(const Matrix4 &transform, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    A_UNUSED(transform);
    A_UNUSED(mesh);
    A_UNUSED(sub);
    A_UNUSED(layer);
    A_UNUSED(material);
}
/*!
    Draws the same \a mesh multiple times using GPU instancing.
    GPU will draw this mesh with the specified \a sub mesh index in different \a transform locations with assigned \a material, and rendering \a layer.
    Parameter \a count specifies the number of instances to draw.
*/
void CommandBuffer::drawMeshInstanced(const Matrix4 *transform, uint32_t count, Mesh *mesh, uint32_t sub, uint32_t layer, MaterialInstance *material) {
    A_UNUSED(transform);
    A_UNUSED(count);
    A_UNUSED(mesh);
    A_UNUSED(sub);
    A_UNUSED(layer);
    A_UNUSED(material);
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
    Sets the \a color for rendering commands.
*/
void CommandBuffer::setColor(const Vector4 &color) {
    m_local.color = color;
}
/*!
    Sets the object \a id for rendering commands.
*/
void CommandBuffer::setObjectId(uint32_t id) {
    m_local.objectId = idToColor(id);
}
/*!
    Sets the material \a id for rendering commands.
*/
void CommandBuffer::setMaterialId(uint32_t id) {
    m_local.materialId = idToColor(id);
}
/*!
    Sets the screen projection matrix.
    Parameters \a x and \a y represents screen coordinates.
    \a width and \a height screen dimensions.
*/
void CommandBuffer::setScreenProjection(float x, float y, float width, float height) {
    if(!m_screenProjection) {
        setViewProjection(Matrix4(), Matrix4::ortho(x, width, y, height, 0.0f, 100.0f));
        m_screenProjection = true;
    }
}
/*!
    Resets the view and projection matrices to their saved values.
*/
void CommandBuffer::resetViewProjection() {
    m_global.view = m_saveView;
    m_global.projection = m_saveProjection;
    m_screenProjection = false;
}
/*!
     Sets the \a view and \a projection matrices.
*/
void CommandBuffer::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_saveView = m_global.view;
    m_saveProjection = m_global.projection;

    m_global.view = view;
    m_global.projection = projection;
}
/*!
     Sets a global \a value based on its \a name.
*/
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
/*!
     Sets a global \a texture based on its \a name.
*/
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
/*!
    Returns current projection matrix.
*/
Matrix4 CommandBuffer::projection() const {
    return m_global.projection;
}
/*!
    Returns current view matrix.
*/
Matrix4 CommandBuffer::view() const {
    return m_global.view;
}
/*!
     Retrieves a global texture based on its \a name.
*/
Texture *CommandBuffer::texture(const char *name) const {
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
void CommandBuffer::beginDebugMarker(const char *name) {
    A_UNUSED(name);
}
/*!
    Ends the current debug marker.
*/
void CommandBuffer::endDebugMarker() {

}
/*!
    Returns Vector2 representing the viewport dimensions.
*/
Vector2 CommandBuffer::viewport() const {
    return Vector2(m_viewportWidth, m_viewportHeight);
}
/*!
    Sets the viewport dimensions.
    Parameters \a x and \a y represents viewport coordinates.
    \a width and \a height viewport dimensions.
*/
void CommandBuffer::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;

    setGlobalValue("camera.screen", Vector4(width, height, 1.0f / (float)width, 1.0f / (float)height));
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
