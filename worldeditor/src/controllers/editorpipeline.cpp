#include "editorpipeline.h"

#include <commandbuffer.h>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/renderable.h>

#include <resources/rendertarget.h>
#include <resources/material.h>
#include <resources/mesh.h>

#include <postprocess/postprocessor.h>

#include <editor/handles.h>

#include "settingsmanager.h"

#include "objectctrl.h"

#include <QVariant>
#include <QColor>
#include <QMenu>
#include <QRegularExpression>

#define SELECT_MAP  "selectMap"
#define DEPTH_MAP   "depthMap"
#define OUTLINE_MAP "outlineMap"
#define OUTDEPTH_MAP "outdepthMap"

#define G_EMISSIVE  "emissiveMap"

#define SEL_TARGET  "objectSelect"
#define OUT_TARGET  "outLine"
#define FINAL_TARGET "finalTarget"

#define OUTLINE     "Outline"

namespace {
    const QString postSettings("Graphics/");
    const QString gridColor("General/Colors/Grid_Color");
    const QString outlineWidth("General/Colors/Outline_Width");
    const QString outlineColor("General/Colors/Outline_Color");
};

class Outline : public PostProcessor {
public:
    Outline() :
        m_width(1.0f) {
        Material *material = Engine::loadResource<Material>(".embedded/outline.mtl");
        if(material) {
            m_material = material->createInstance();
            m_material->setFloat("width", &m_width);
            m_material->setVector4("color", &m_color);
        }

        m_resultTexture = Engine::objectCreate<Texture>();
        m_resultTexture->setFormat(Texture::RGBA8);

        m_resultTarget->setColorAttachment(0, m_resultTexture);
    }

    float m_width;
    Vector4 m_color;
};

EditorPipeline::EditorPipeline() :
        Pipeline(),
        m_pTarget(nullptr),
        m_pGrid(nullptr),
        m_pGizmo(nullptr),
        m_pController(nullptr),
        m_pOutline(new Outline()),
        m_pDepth(nullptr),
        m_pSelect(nullptr),
        m_ObjectId(0),
        m_MouseX(0),
        m_MouseY(0),
        m_postMenu(nullptr),
        m_lightMenu(nullptr),
        m_bufferMenu(nullptr) {

    {
        Texture *select = Engine::objectCreate<Texture>();
        select->setFormat(Texture::RGBA8);
        m_textureBuffers[SELECT_MAP] = select;
        m_Buffer->setGlobalTexture(SELECT_MAP, select);
    }
    {
        Texture *depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
        m_textureBuffers[OUTDEPTH_MAP] = depth;
        m_Buffer->setGlobalTexture(OUTDEPTH_MAP, depth);
    }
    {
        Texture *outline = Engine::objectCreate<Texture>();
        outline->setFormat(Texture::RGBA8);
        m_textureBuffers[OUTLINE_MAP] = outline;
        m_Buffer->setGlobalTexture(OUTLINE_MAP, outline);
    }

    m_pSelect = Engine::objectCreate<Texture>();
    m_pSelect->setFormat(Texture::RGBA8);
    m_pSelect->resize(1, 1);

    m_pDepth = Engine::objectCreate<Texture>();
    m_pDepth->setFormat(Texture::Depth);
    m_pDepth->setDepthBits(24);
    m_pDepth->setWidth(1);
    m_pDepth->setHeight(1);

    RenderTarget *object = Engine::objectCreate<RenderTarget>();
    object->setColorAttachment(0, m_textureBuffers[SELECT_MAP]);
    object->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[SEL_TARGET] = object;

    RenderTarget *out = Engine::objectCreate<RenderTarget>();
    out->setColorAttachment(0, m_textureBuffers[OUTLINE_MAP]);
    out->setDepthAttachment(m_textureBuffers[OUTDEPTH_MAP]);
    m_renderTargets[OUT_TARGET] = out;

    RenderTarget *final = Engine::objectCreate<RenderTarget>();
    final->setColorAttachment(0, m_pFinal);
    final->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[FINAL_TARGET] = final;

    m_pGrid = Engine::objectCreate<Mesh>("Grid");

    m_PostEffects.push_back(m_pOutline);

    Lod lod;
    Vector3Vector &vertices = lod.vertices();
    IndexVector &indices = lod.indices();
    vertices.resize(404);
    indices.resize(404);
    for(uint8_t x = 0; x <= 100; x++) {
        uint32_t index = x * 2;
        vertices[index] = Vector3(x - 50, -50, 0);
        vertices[index + 1] = Vector3(x - 50, 50, 0);

        indices[index] = index;
        indices[index + 1] = index + 1;
    }
    for(uint8_t y = 0; y <= 100; y++) {
        uint32_t index = y * 2 + 202;
        vertices[index] = Vector3(-50, y - 50, 0);
        vertices[index + 1] = Vector3(50, y - 50, 0);

        indices[index] = index;
        indices[index + 1] = index + 1;
    }

    m_pGrid->setTopology(Mesh::Lines);
    m_pGrid->addLod(&lod);

    Material *m = Engine::loadResource<Material>(".embedded/gizmo.mtl");
    if(m) {
        m_pGizmo = m->createInstance();
    }

    Handles::init();

    SettingsManager::instance()->registerProperty(qPrintable(gridColor), QColor(102, 102, 102, 102));
    SettingsManager::instance()->registerProperty(qPrintable(outlineWidth), 1.0f);
    SettingsManager::instance()->registerProperty(qPrintable(outlineColor), QColor(255, 128, 0, 255));

    for(auto it : m_PostEffects) {
        SettingsManager::instance()->registerProperty(qPrintable(postSettings + it->name()), it->isEnabled());
    }

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &EditorPipeline::onApplySettings);
}

void EditorPipeline::onApplySettings() {
    QColor color = SettingsManager::instance()->property(qPrintable(gridColor)).value<QColor>();
    m_SecondaryGridColor = m_PrimaryGridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());

    color = SettingsManager::instance()->property(qPrintable(outlineColor)).value<QColor>();
    m_pOutline->m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    m_pOutline->m_width = SettingsManager::instance()->property(qPrintable(outlineWidth)).toFloat();

    for(auto it : m_PostEffects) {
        it->setEnabled(SettingsManager::instance()->property(qPrintable(postSettings + it->name())).toBool());
    }
}

void EditorPipeline::setController(CameraCtrl *ctrl) {
    m_pController = ctrl;
}

uint32_t EditorPipeline::objectId() const {
    return m_ObjectId;
}

Vector3 EditorPipeline::mouseWorld () const {
    return m_MouseWorld;
}

void EditorPipeline::setMousePosition(int32_t x, int32_t y) {
    m_MouseX = x;
    m_MouseY = y;
}

void EditorPipeline::setDragObjects(const ObjectList &list) {
    m_DragList.clear();
    for(auto it : list) {
        auto result = it->findChildren<Renderable *>();

        m_DragList.insert(m_DragList.end(), result.begin(), result.end());
    }
}

void EditorPipeline::setTarget(const QString &string) {
    m_pTarget = nullptr;
    auto it = m_textureBuffers.find(qPrintable(string));
    if(it != m_textureBuffers.end()) {
        m_pTarget = it->second;
    }
}

QStringList EditorPipeline::targets() const {
    QStringList result;
    for(auto &it : m_textureBuffers) {
        result.push_back(it.first.c_str());
    }

    return result;
}

void EditorPipeline::createMenu(QMenu *menu) {
    m_postMenu = menu->addMenu(tr("Post Processing"));
    m_lightMenu = menu->addMenu(tr("Lighting Features"));
    m_bufferMenu = menu->addMenu(tr("Buffer Visualization"));

    fillEffectMenu(m_lightMenu, ICommandBuffer::DEFAULT | ICommandBuffer::LIGHT);
    fillEffectMenu(m_postMenu, ICommandBuffer::TRANSLUCENT | ICommandBuffer::UI);

    QObject::connect(m_bufferMenu, &QMenu::aboutToShow, this, &EditorPipeline::onBufferMenu);
}

void EditorPipeline::draw(Camera &camera) {
    // Retrive object id
    m_Buffer->setRenderTarget(m_renderTargets[SEL_TARGET]);
    m_Buffer->clearRenderTarget();

    m_Buffer->setViewport(0, 0, m_Width, m_Height);

    cameraReset(camera);
    drawComponents(ICommandBuffer::RAYCAST, m_Filter);
    drawComponents(ICommandBuffer::RAYCAST, m_UiComponents);

    Vector3 screen((float)m_MouseX / (float)m_Width, (float)m_MouseY / (float)m_Height, 0.0f);

    m_pSelect->readPixels(m_MouseX, m_MouseY, 1, 1);
    m_ObjectId = m_pSelect->getPixel(0, 0);
    if(m_ObjectId) {
        m_pDepth->readPixels(m_MouseX, m_MouseY, 1, 1);
        uint32_t pixel = m_pDepth->getPixel(0, 0);
        memcpy(&screen.z, &pixel, sizeof(float));

        m_MouseWorld = Camera::unproject(screen, camera.viewMatrix(), camera.projectionMatrix());
    } else {
        Ray ray = camera.castRay(screen.x, screen.y);
        m_MouseWorld = (ray.dir * 10.0f) + ray.pos;
    }
    for(auto it : m_DragList) {
        it->update();
        m_Filter.push_back(it);
    }

    // Selection outline
    m_Buffer->setRenderTarget(m_renderTargets[OUT_TARGET]);
    m_Buffer->clearRenderTarget();
    RenderList filter;
    for(auto actor : m_pController->selected()) {
        for(auto it : m_Filter) {
            Renderable *component = dynamic_cast<Renderable *>(it);
            if(component && isInHierarchy(component->actor(), static_cast<Actor *>(actor))) {
                filter.push_back(component);
            }
        }
    }
    drawComponents(ICommandBuffer::RAYCAST, filter);

    Pipeline::draw(camera);

    if(m_pTarget != nullptr) {
        m_pFinal = m_pTarget;
    }

    m_renderTargets[FINAL_TARGET]->setColorAttachment(0, m_pFinal);

    if(m_pTarget == nullptr) {
        // Draw handles
        cameraReset(camera);
        m_Buffer->setRenderTarget(m_renderTargets[FINAL_TARGET]);
        drawGrid(camera);

        Handles::beginDraw(m_Buffer);
        m_pController->drawHandles();
        Handles::endDraw();
    }
}

void EditorPipeline::drawUi(Camera &camera) {
    cameraReset(camera);
    drawComponents(ICommandBuffer::UI | ICommandBuffer::TRANSLUCENT, m_UiComponents);

    postProcess(m_renderTargets["lightPass"], ICommandBuffer::UI);
}

bool EditorPipeline::isInHierarchy(Actor *origin, Actor *actor) {
    if(origin == actor) {
        return true;
    }
    Actor *parent = static_cast<Actor *>(origin->parent());
    if(parent) {
        return isInHierarchy(parent, actor);
    }

    return false;
}

void EditorPipeline::resize(int32_t width, int32_t height) {
    Pipeline::resize(width, height);
    m_pController->resize(width, height);
}

void EditorPipeline::drawGrid(Camera &camera) {
    Transform *t = camera.actor()->transform();
    Vector3 cam = t->position();
    Vector3 pos(cam.x, 0.0f, cam.z);
    float length = (pos - cam).length();

    bool ortho = camera.orthographic();

    if(ortho) {
        float depth = camera.farPlane() - camera.nearPlane();
        switch(m_pController->viewSide()) {
            case CameraCtrl::ViewSide::VIEW_FRONT: {
                pos = Vector3(cam.x, cam.y, cam.z - depth);
            } break;
            case CameraCtrl::ViewSide::VIEW_BACK: {
                pos = Vector3(cam.x, cam.y, cam.z + depth);
            } break;
            case CameraCtrl::ViewSide::VIEW_LEFT: {
                pos = Vector3(cam.x + depth, cam.y, cam.z);
            } break;
            case CameraCtrl::ViewSide::VIEW_RIGHT: {
                pos = Vector3(cam.x - depth, cam.y, cam.z);
            } break;
            case CameraCtrl::ViewSide::VIEW_TOP: {
                pos = Vector3(cam.x, cam.y - depth, cam.z);
            } break;
            case CameraCtrl::ViewSide::VIEW_BOTTOM: {
                pos = Vector3(cam.x, cam.y + depth, cam.z);
            } break;
            default: break;
        }

        length = camera.orthoSize();
    }

    float scale = 0.001f;
    while(scale < length) {
        scale *= 10.0f;
    }

    float factor = 1.0f - (length / scale);
    m_SecondaryGridColor.w = m_PrimaryGridColor.w * factor;

    if(ortho) {
        scale *= 0.2f;
    }

    pos = Vector3(scale * int32_t(pos.x / scale),
                  0.0f,
                  scale * int32_t(pos.z / scale));

    Quaternion rot;
    if(ortho) {
        switch(m_pController->viewSide()) {
            case CameraCtrl::ViewSide::VIEW_FRONT:
            case CameraCtrl::ViewSide::VIEW_BACK: {
                rot = Quaternion();
                pos = Vector3(scale * int32_t(pos.x / scale),
                              scale * int32_t(pos.y / scale),
                              pos.z);
            } break;
            case CameraCtrl::ViewSide::VIEW_LEFT:
            case CameraCtrl::ViewSide::VIEW_RIGHT: {
                rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                pos = Vector3(pos.x,
                              scale * int32_t(pos.y / scale),
                              scale * int32_t(pos.z / scale));
            } break;
            case CameraCtrl::ViewSide::VIEW_TOP:
            case CameraCtrl::ViewSide::VIEW_BOTTOM:
            default: {
                rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                pos = Vector3(scale * int32_t(pos.x / scale),
                              pos.y,
                              scale * int32_t(pos.z / scale));
            } break;
        }
    } else {
        pos = Vector3(scale * int32_t(pos.x / scale),
                      0.0f,
                      scale * int32_t(pos.z / scale));

        rot = Quaternion(Vector3(1, 0, 0), 90.0f);
    }

    Matrix4 transform(pos, rot, scale);

    m_Buffer->setColor(m_PrimaryGridColor);
    m_Buffer->drawMesh(transform, m_pGrid, ICommandBuffer::TRANSLUCENT, m_pGizmo);

    Matrix4 m;
    m.scale(0.1f);

    m_Buffer->setColor(m_SecondaryGridColor);
    m_Buffer->drawMesh(transform * m, m_pGrid, ICommandBuffer::TRANSLUCENT, m_pGizmo);

    m_Buffer->setColor(Vector4(1.0f));
}

void EditorPipeline::onBufferMenu() {
    if(m_bufferMenu) {
        m_bufferMenu->clear();

        QStringList list = targets();
        list.push_front(tr("Final Buffer"));

        bool first = true;
        for(auto &it : list) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result = it;
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = m_bufferMenu->addAction(result);
            action->setData(it);
            QObject::connect(action, &QAction::triggered, this, &EditorPipeline::onBufferChanged);
            if(first) {
                m_bufferMenu->addSeparator();
                first = false;
            }
        }
    }
}

void EditorPipeline::fillEffectMenu(QMenu *menu, uint32_t layers) {
    if(menu) {
        menu->clear();

        for(auto &it : m_PostEffects) {
            if(it->layer() & layers) {
                static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
                static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

                QString result = it->name();
                if(result.isEmpty()) {
                    continue;
                }
                result.replace(regExp1, "\\1 \\2");
                result.replace(regExp2, "\\1 \\2");
                result.replace(0, 1, result[0].toUpper());

                QAction *action = menu->addAction(result);
                action->setCheckable(true);
                action->setChecked(it->isEnabled());
                action->setData(it->name());

                QObject::connect(action, &QAction::toggled, this, &EditorPipeline::onPostEffectChanged);
            }
        }
    }
}

void EditorPipeline::onBufferChanged() {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        setTarget(action->data().toString());
    }
}

void EditorPipeline::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        for(auto &it : m_PostEffects) {
            if(action->data().toString() == it->name()) {
                it->setEnabled(checked);
                SettingsManager::instance()->setProperty(qPrintable(postSettings + it->name()), checked);
            }
        }
    }
}
