#ifndef EDITORPIPELINE_H
#define EDITORPIPELINE_H

#include "pipeline.h"

#include <QObject>

class CameraCtrl;
class Texture;
class Renderable;
class Outline;

class QMenu;

class ENGINE_EXPORT EditorPipeline : public QObject, public Pipeline {
public:
    EditorPipeline();

    void setController(CameraCtrl *ctrl);

    uint32_t objectId() const;

    Vector3 mouseWorld() const;

    void setMousePosition(int32_t x, int32_t y);

    void setDragObjects(const ObjectList &list);

    void debugRenderTexture(const QString &string = QString());
    QStringList renderTextures() const;

    void createMenu(QMenu *menu);

    static void registerSettings();

private slots:
    void onBufferMenu();

    void onBufferChanged();
    void onPostEffectChanged(bool checked);

    void onApplySettings();

protected:
    void fillEffectMenu(QMenu *menu, uint32_t layers);

    void drawGrid(Camera &camera);

    void draw(Camera &camera) override;

    void resize(int32_t width, int32_t height) override;

    void drawUi(Camera &camera) override;

    bool isInHierarchy(Actor *origin, Actor *actor);

    Texture *m_pTarget;

    Vector4 m_PrimaryGridColor;
    Vector4 m_SecondaryGridColor;

    Vector3 m_MouseWorld;

    Mesh *m_pGrid;

    MaterialInstance *m_pGizmo;

    CameraCtrl *m_pController;

    Outline *m_pOutline;

    list<Renderable *> m_DragList;

    Texture *m_pDepth;
    Texture *m_pSelect;

    uint32_t m_ObjectId;
    int32_t m_MouseX;
    int32_t m_MouseY;

    QMenu *m_postMenu;
    QMenu *m_lightMenu;
    QMenu *m_bufferMenu;
};

#endif // EDITORPIPELINE_H
