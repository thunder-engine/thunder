#ifndef AGL_H
#define AGL_H

#if THUNDER_MOBILE
    #if defined(__ANDROID__)
        #define GLFM_INCLUDE_ES31
    #else
        #define GLFM_INCLUDE_ES3

        #define glProgramUniform1i  glProgramUniform1iEXT
        #define glProgramUniform1f  glProgramUniform1fEXT

        #define glProgramUniform1iv glProgramUniform1ivEXT
        #define glProgramUniform1fv glProgramUniform1fvEXT
        #define glProgramUniform2fv glProgramUniform2fvEXT
        #define glProgramUniform3fv glProgramUniform3fvEXT
        #define glProgramUniform4fv glProgramUniform4fvEXT

        #define glProgramUniformMatrix4fv   glProgramUniformMatrix4fvEXT
        #define glProgramUniformMatrix3fv   glProgramUniformMatrix3fvEXT

        #define glGenProgramPipelines       glGenProgramPipelinesEXT
        #define glDeleteProgramPipelines    glDeleteProgramPipelinesEXT

        #define glUseProgramStages      glUseProgramStagesEXT
        #define glBindProgramPipeline   glBindProgramPipelineEXT

        #define glProgramParameteri     glProgramParameteriEXT

        #define GL_PROGRAM_SEPARABLE    GL_PROGRAM_SEPARABLE_EXT
        #define GL_VERTEX_SHADER_BIT    GL_VERTEX_SHADER_BIT_EXT
        #define GL_FRAGMENT_SHADER_BIT  GL_FRAGMENT_SHADER_BIT_EXT

    #endif
    #include <glfm.h>
#else
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
#endif

#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

#endif // AGL_H
