#ifndef EDITORPIPELINE_H
#define EDITORPIPELINE_H

#include "pipelinecontext.h"

#include <QObject>

class CameraCtrl;
class Texture;
class Renderable;
class Outline;

class ENGINE_EXPORT EditorPipeline : public QObject, public PipelineContext {
public:
    EditorPipeline();

    void setController(CameraCtrl *ctrl);

    uint32_t objectId() const;

    Vector3 mouseWorld() const;

    void setMousePosition(int32_t x, int32_t y);

    void setDragObjects(const ObjectList &list);

    static void registerSettings();

private slots:
    void onApplySettings();

protected:
    void draw(Camera &camera) override;

    void drawUi(Camera &camera) override;

    void drawGrid(Camera &camera);

    Vector3 m_mouseWorld;

    Vector4 m_gridColor;

    list<Renderable *> m_dragList;

    CameraCtrl *m_controller;

    Outline *m_outline;

    Texture *m_depth;

    MaterialInstance *m_grid;

    uint32_t m_objectId;
    int32_t m_mouseX;
    int32_t m_mouseY;
};

#endif // EDITORPIPELINE_H
