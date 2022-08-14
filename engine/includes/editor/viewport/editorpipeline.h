#ifndef EDITORPIPELINE_H
#define EDITORPIPELINE_H

#include "pipelinecontext.h"

#include <QObject>

class CameraCtrl;
class Outline;

class ENGINE_EXPORT EditorPipeline : public QObject, public PipelineContext {
public:
    EditorPipeline();

    void setController(CameraCtrl *ctrl);

private slots:
    void onApplySettings();

protected:
    void draw(Camera &camera) override;

    void drawUi(Camera &camera) override;

    void drawGrid(Camera &camera);

    Vector4 m_gridColor;

    CameraCtrl *m_controller;

    MaterialInstance *m_grid;

};

#endif // EDITORPIPELINE_H
