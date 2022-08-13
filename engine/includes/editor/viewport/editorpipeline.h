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

    MaterialInstance *m_grid;

};

#endif // EDITORPIPELINE_H
