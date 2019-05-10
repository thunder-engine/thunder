#ifndef OBJECTCONTROLLERPIPELINE_H
#define OBJECTCONTROLLERPIPELINE_H

#include "pipeline.h"

#include <QObject>

class ObjectCtrlPipeline : public Pipeline {

public:
    ObjectCtrlPipeline();

    void draw(Scene *scene, Camera &camera);

    void loadSettings();

protected:
    void drawGrid(Camera &camera);

    Vector4 m_PrimaryGridColor;
    Vector4 m_SecondaryGridColor;

    Mesh *m_pGrid;

    MaterialInstance *m_pGizmo;

};

#endif // OBJECTCONTROLLERPIPELINE_H
