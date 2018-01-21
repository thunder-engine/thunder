/*
 GLFM
 https://github.com/brackeen/glfm
 Copyright (c) 2014-2017 David Brackeen
 
 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the
 use of this software. Permission is granted to anyone to use this software
 for any purpose, including commercial applications, and to alter it and
 redistribute it freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef GLFM_PLATFORM_H
#define GLFM_PLATFORM_H

#include "glfm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GLFMDisplay {
    // Config
    GLFMRenderingAPI preferredAPI;
    GLFMColorFormat colorFormat;
    GLFMDepthFormat depthFormat;
    GLFMStencilFormat stencilFormat;
    GLFMMultisample multisample;
    GLFMUserInterfaceOrientation allowedOrientations;
    GLFMUserInterfaceChrome uiChrome;

    // Callbacks
    GLFMMainLoopFunc mainLoopFunc;
    GLFMTouchFunc touchFunc;
    GLFMKeyFunc keyFunc;
    GLFMCharFunc charFunc;
    GLFMSurfaceErrorFunc surfaceErrorFunc;
    GLFMSurfaceCreatedFunc surfaceCreatedFunc;
    GLFMSurfaceResizedFunc surfaceResizedFunc;
    GLFMSurfaceDestroyedFunc surfaceDestroyedFunc;
    GLFMKeyboardVisibilityChangedFunc keyboardVisibilityChangedFunc;
    GLFMMemoryWarningFunc lowMemoryFunc;
    GLFMAppFocusFunc focusFunc;

    // External data
    void *userData;
    void *platformData;
};

// MARK: Setters

void glfmSetSurfaceErrorFunc(GLFMDisplay *display, GLFMSurfaceErrorFunc surfaceErrorFunc) {
    if (display) {
        display->surfaceErrorFunc = surfaceErrorFunc;
    }
}

void glfmSetDisplayConfig(GLFMDisplay *display,
                          const GLFMRenderingAPI preferredAPI,
                          const GLFMColorFormat colorFormat,
                          const GLFMDepthFormat depthFormat,
                          const GLFMStencilFormat stencilFormat,
                          const GLFMMultisample multisample) {
    if (display) {
        display->preferredAPI = preferredAPI;
        display->colorFormat = colorFormat;
        display->depthFormat = depthFormat;
        display->stencilFormat = stencilFormat;
        display->multisample = multisample;
    }
}

GLFMUserInterfaceOrientation glfmGetUserInterfaceOrientation(GLFMDisplay *display) {
    return display ? display->allowedOrientations : GLFMUserInterfaceOrientationAny;
}

void glfmSetUserData(GLFMDisplay *display, void *userData) {
    if (display) {
        display->userData = userData;
    }
}

void *glfmGetUserData(GLFMDisplay *display) {
    return display ? display->userData : NULL;
}

void _glfmDisplayChromeUpdated(GLFMDisplay *display);

GLFMUserInterfaceChrome glfmGetDisplayChrome(GLFMDisplay *display) {
    return display ? display->uiChrome : GLFMUserInterfaceChromeNavigation;
}

void glfmSetDisplayChrome(GLFMDisplay *display, GLFMUserInterfaceChrome uiChrome) {
    if (display) {
        display->uiChrome = uiChrome;
        _glfmDisplayChromeUpdated(display);
    }
}

void glfmSetMainLoopFunc(GLFMDisplay *display, GLFMMainLoopFunc mainLoopFunc) {
    if (display) {
        display->mainLoopFunc = mainLoopFunc;
    }
}

void glfmSetSurfaceCreatedFunc(GLFMDisplay *display, GLFMSurfaceCreatedFunc surfaceCreatedFunc) {
    if (display) {
        display->surfaceCreatedFunc = surfaceCreatedFunc;
    }
}

void glfmSetSurfaceResizedFunc(GLFMDisplay *display, GLFMSurfaceResizedFunc surfaceResizedFunc) {
    if (display) {
        display->surfaceResizedFunc = surfaceResizedFunc;
    }
}

void glfmSetSurfaceDestroyedFunc(GLFMDisplay *display,
                                 GLFMSurfaceDestroyedFunc surfaceDestroyedFunc) {
    if (display) {
        display->surfaceDestroyedFunc = surfaceDestroyedFunc;
    }
}

void glfmSetKeyboardVisibilityChangedFunc(GLFMDisplay *display,
                                          GLFMKeyboardVisibilityChangedFunc func) {
    if (display) {
        display->keyboardVisibilityChangedFunc = func;
    }
}

void glfmSetTouchFunc(GLFMDisplay *display, GLFMTouchFunc touchFunc) {
    if (display) {
        display->touchFunc = touchFunc;
    }
}

void glfmSetKeyFunc(GLFMDisplay *display, GLFMKeyFunc keyFunc) {
    if (display) {
        display->keyFunc = keyFunc;
    }
}

void glfmSetCharFunc(GLFMDisplay *display, GLFMCharFunc charFunc) {
    if (display) {
        display->charFunc = charFunc;
    }
}

void glfmSetMemoryWarningFunc(GLFMDisplay *display, GLFMMemoryWarningFunc lowMemoryFunc) {
    if (display) {
        display->lowMemoryFunc = lowMemoryFunc;
    }
}

void glfmSetAppFocusFunc(GLFMDisplay *display, GLFMAppFocusFunc focusFunc) {
    if (display) {
        display->focusFunc = focusFunc;
    }
}

// MARK: Helper functions

static void _glfmReportSurfaceError(GLFMDisplay *display, const char *errorMessage) {
    if (display->surfaceErrorFunc && errorMessage) {
        display->surfaceErrorFunc(display, errorMessage);
    }
}

// glfmExtensionSupported function is from
// http://www.opengl.org/archives/resources/features/OGLextensions/
bool glfmExtensionSupported(const char *extension) {
    // Extension names should not have spaces.
    GLubyte *where = (GLubyte *)strchr(extension, ' ');
    if (where || *extension == '\0') {
        return false;
    }

    const GLubyte *extensions = glGetString(GL_EXTENSIONS);

    // It takes a bit of care to be fool-proof about parsing the
    // OpenGL extensions string. Don't be fooled by sub-strings, etc.
    const GLubyte *start = extensions;
    for (;;) {
        where = (GLubyte *)strstr((const char *)start, extension);
        if (!where) {
            break;
        }

        GLubyte *terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ') {
            if (*terminator == ' ' || *terminator == '\0') {
                return true;
            }
        }

        start = terminator;
    }

    return false;
}

#ifdef __cplusplus
}
#endif

#endif
