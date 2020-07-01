#ifndef OBJECTCONTROLLERPIPELINE_H
#define OBJECTCONTROLLERPIPELINE_H

#include "pipeline.h"

#include <QObject>

class ObjectCtrl;
class Texture;

class ObjectCtrlPipeline : public Pipeline {
public:
    ObjectCtrlPipeline();

    void draw(Camera &camera) override;

    void post(Camera &camera) override;

    void resize(int32_t width, int32_t height) override;

    void loadSettings();

    void setController(ObjectCtrl *ctrl);

    uint32_t objectId () const;

    Vector3 mouseWorld () const;

    void setMousePosition(int32_t x, int32_t y);

    void setDragObjects(const ObjectList &list);

protected:
    void drawGrid(Camera &camera);

    Vector4 m_PrimaryGridColor;
    Vector4 m_SecondaryGridColor;

    Vector3 m_MouseWorld;

    Mesh *m_pGrid;

    MaterialInstance *m_pGizmo;

    ObjectCtrl *m_pController;

    ObjectList m_DragList;

    Texture *m_pDepth;
    Texture *m_pSelect;

    uint32_t m_ObjectId;
    int32_t m_MouseX;
    int32_t m_MouseY;
};

#endif // OBJECTCONTROLLERPIPELINE_H
