#include "postprocess/aantialiasinggl.h"

#include "engine.h"

#include "apipeline.h"

AAntiAliasingGL::AAntiAliasingGL() {
    reset("shaders/SSAA.frag");

}

