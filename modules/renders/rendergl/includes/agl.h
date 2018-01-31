#ifndef AGL_H
#define AGL_H

#ifdef iOS
    #if (ES == 2)
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#endif

#if defined(__ANDROID__)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif

#ifdef __APPLE__
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#elif _WIN32
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif

#define VERTICES    "Vertices"
#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

#endif // AGL_H
