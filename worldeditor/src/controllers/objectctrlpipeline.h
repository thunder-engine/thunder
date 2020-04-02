#ifndef OBJECTCONTROLLERPIPELINE_H
#define OBJECTCONTROLLERPIPELINE_H

#include "pipeline.h"

#include <QObject>

class ObjectCtrl;

class ObjectCtrlPipeline : public Pipeline {
public:
    ObjectCtrlPipeline();

    void draw(Camera &camera) override;

    void post(Camera &camera) override;

    void resize(int32_t width, int32_t height) override;

    void loadSettings();

    void setController(ObjectCtrl *ctrl);

protected:
    void drawGrid(Camera &camera);

    Vector4 m_PrimaryGridColor;
    Vector4 m_SecondaryGridColor;

    Mesh *m_pGrid;

    MaterialInstance *m_pGizmo;

    ObjectCtrl *m_pController;
};

#endif // OBJECTCONTROLLERPIPELINE_H
