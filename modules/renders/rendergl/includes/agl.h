#ifndef AGL_H
#define AGL_H

#if defined(THUNDER_MOBILE)
    #define GLFM_INCLUDE_ES3

    #include <glfm.h>
#else
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif

#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

void _CheckGLError(const char *file, int line);
#define CheckGLError() //_CheckGLError(__FILE__, __LINE__)

#endif // AGL_H
