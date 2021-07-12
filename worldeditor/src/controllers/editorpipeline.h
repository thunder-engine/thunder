#ifndef OBJECTCONTROLLERPIPELINE_H
#define OBJECTCONTROLLERPIPELINE_H

#include "pipeline.h"

#include <QObject>

class CameraCtrl;
class Texture;
class Renderable;
class Outline;

class EditorPipeline : public Pipeline {
public:
    EditorPipeline();

    void loadSettings();

    void setController(CameraCtrl *ctrl);

    uint32_t objectId () const;

    Vector3 mouseWorld () const;

    void setMousePosition(int32_t x, int32_t y);

    void setDragObjects(const ObjectList &list);

    void setTarget(const QString &string = QString());
    QStringList targets() const;

protected:
    void drawGrid(Camera &camera);

    void draw(Camera &camera) override;

    void resize(int32_t width, int32_t height) override;

    void drawUi(Camera &camera) override;

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
};

#endif // OBJECTCONTROLLERPIPELINE_H
