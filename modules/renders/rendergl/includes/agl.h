#ifndef AGL_H
#define AGL_H

#ifdef iOS
    #if (ES == 2)
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#endif

#if defined(__ANDROID__)
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

#endif // AGL_H
