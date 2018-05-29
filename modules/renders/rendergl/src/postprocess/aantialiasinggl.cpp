#include "postprocess/aantialiasinggl.h"

#include "engine.h"

#include "pipeline.h"

AAntiAliasingGL::AAntiAliasingGL() {
    reset("shaders/SSAA.frag");

}

