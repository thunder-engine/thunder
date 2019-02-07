#ifndef OBJECTCONTROLLERPIPELINE_H
#define OBJECTCONTROLLERPIPELINE_H

#include "pipeline.h"

class ObjectCtrlPipeline : public Pipeline {
public:
    ObjectCtrlPipeline();

    void draw(Scene *scene, Camera &camera);
};

#endif // OBJECTCONTROLLERPIPELINE_H
