#ifndef AGL_H
#define AGL_H

#if THUNDER_MOBILE
    #define GLFM_INCLUDE_ES3
    #include <glfm.h>
#else
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif

#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

#endif // AGL_H
