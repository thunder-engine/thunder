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

#include "glfm.h"

#ifdef GLFM_PLATFORM_ANDROID

#include "android_native_app_glue.h"
#include "glfm_platform.h"
#include <EGL/egl.h>
#include <android/log.h>
#include <android/window.h>
#include <dlfcn.h>
#include <unistd.h>

#ifdef NDEBUG
#define LOG_DEBUG(...) do { } while (0)
#else
#define LOG_DEBUG(...) __android_log_print(ANDROID_LOG_INFO, "GLFM", __VA_ARGS__)
#endif

//#define LOG_LIFECYCLE(...) __android_log_print(ANDROID_LOG_INFO, "GLFM", __VA_ARGS__)
#define LOG_LIFECYCLE(...) do { } while (0)

// MARK: Time utils

static struct timespec _glfmTimeNow() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return t;
}

static double _glfmTimeSeconds(struct timespec t) {
    return t.tv_sec + (double)t.tv_nsec / 1e9;
}

static struct timespec _glfmTimeSubstract(struct timespec a, struct timespec b) {
    struct timespec result;
    if (b.tv_nsec > a.tv_nsec) {
        result.tv_sec = a.tv_sec - b.tv_sec - 1;
        result.tv_nsec = 1000000000 - b.tv_nsec + a.tv_nsec;
    } else {
        result.tv_sec = a.tv_sec - b.tv_sec;
        result.tv_nsec = a.tv_nsec - b.tv_nsec;
    }
    return result;
}

// MARK: Platform data (global singleton)

#define MAX_SIMULTANEOUS_TOUCHES 5

typedef struct {
    struct android_app *app;

    bool multitouchEnabled;

    ARect keyboardFrame;
    bool keyboardVisible;

    struct timespec initTime;
    bool animating;
    bool hasInited;

    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLConfig eglConfig;
    EGLContext eglContext;
    bool eglContextCurrent;

    int32_t width;
    int32_t height;
    double scale;

    GLFMDisplay *display;
    GLFMRenderingAPI renderingAPI;

    JNIEnv *jniEnv;
} GLFMPlatformData;

static GLFMPlatformData *platformDataGlobal = NULL;

// MARK: JNI code

#define _glfmWasJavaExceptionThrown() \
    ((*jni)->ExceptionCheck(jni) ? ((*jni)->ExceptionClear(jni), true) : false)

#define _glfmClearJavaException() \
    if ((*jni)->ExceptionCheck(jni)) { \
        (*jni)->ExceptionClear(jni); \
    }

static jmethodID _glfmGetJavaMethodID(JNIEnv *jni, jobject object, const char *name,
                                      const char *sig) {
    if (object) {
        jclass class = (*jni)->GetObjectClass(jni, object);
        jmethodID methodID = (*jni)->GetMethodID(jni, class, name, sig);
        (*jni)->DeleteLocalRef(jni, class);
        return _glfmWasJavaExceptionThrown() ? NULL : methodID;
    } else {
        return NULL;
    }
}

static jfieldID _glfmGetJavaFieldID(JNIEnv *jni, jobject object, const char *name, const char *sig) {
    if (object) {
        jclass class = (*jni)->GetObjectClass(jni, object);
        jfieldID fieldID = (*jni)->GetFieldID(jni, class, name, sig);
        (*jni)->DeleteLocalRef(jni, class);
        return _glfmWasJavaExceptionThrown() ? NULL : fieldID;
    } else {
        return NULL;
    }
}

static jfieldID _glfmGetJavaStaticFieldID(JNIEnv *jni, jclass class, const char *name,
                                          const char *sig) {
    if (class) {
        jfieldID fieldID = (*jni)->GetStaticFieldID(jni, class, name, sig);
        return _glfmWasJavaExceptionThrown() ? NULL : fieldID;
    } else {
        return NULL;
    }
}

#define _glfmCallJavaMethod(jni, object, methodName, methodSig, returnType) \
    (*jni)->Call##returnType##Method(jni, object, \
        _glfmGetJavaMethodID(jni, object, methodName, methodSig))

#define _glfmCallJavaMethodWithArgs(jni, object, methodName, methodSig, returnType, ...) \
    (*jni)->Call##returnType##Method(jni, object, \
        _glfmGetJavaMethodID(jni, object, methodName, methodSig), __VA_ARGS__)

#define _glfmGetJavaField(jni, object, fieldName, fieldSig, fieldType) \
    (*jni)->Get##fieldType##Field(jni, object, \
        _glfmGetJavaFieldID(jni, object, fieldName, fieldSig))

#define _glfmGetJavaStaticField(jni, class, fieldName, fieldSig, fieldType) \
    (*jni)->GetStatic##fieldType##Field(jni, class, \
        _glfmGetJavaStaticFieldID(jni, class, fieldName, fieldSig))

static void _glfmSetOrientation(struct android_app *app) {
    static const int ActivityInfo_SCREEN_ORIENTATION_SENSOR = 0x00000004;
    static const int ActivityInfo_SCREEN_ORIENTATION_SENSOR_LANDSCAPE = 0x00000006;
    static const int ActivityInfo_SCREEN_ORIENTATION_SENSOR_PORTRAIT = 0x00000007;

    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    int orientation;
    switch (platformData->display->allowedOrientations) {
        case GLFMUserInterfaceOrientationPortrait:
            orientation = ActivityInfo_SCREEN_ORIENTATION_SENSOR_PORTRAIT;
            break;
        case GLFMUserInterfaceOrientationLandscape:
            orientation = ActivityInfo_SCREEN_ORIENTATION_SENSOR_LANDSCAPE;
            break;
        case GLFMUserInterfaceOrientationAny:
        default:
            orientation = ActivityInfo_SCREEN_ORIENTATION_SENSOR;
            break;
    }

    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return;
    }

    _glfmCallJavaMethodWithArgs(jni, app->activity->clazz, "setRequestedOrientation", "(I)V", Void,
                                orientation);
    _glfmClearJavaException()
}

static jobject _glfmGetDecorView(struct android_app *app) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return NULL;
    }
    jobject window = _glfmCallJavaMethod(jni, app->activity->clazz, "getWindow",
                                         "()Landroid/view/Window;", Object);
    if (!window || _glfmWasJavaExceptionThrown()) {
        return NULL;
    }
    jobject decorView = _glfmCallJavaMethod(jni, window, "getDecorView", "()Landroid/view/View;",
                                            Object);
    (*jni)->DeleteLocalRef(jni, window);
    return _glfmWasJavaExceptionThrown() ? NULL : decorView;
}

static void _glfmSetFullScreen(struct android_app *app, GLFMUserInterfaceChrome uiChrome) {
    static const int View_STATUS_BAR_HIDDEN = 0x00000001;
    static const int View_SYSTEM_UI_FLAG_LOW_PROFILE = 0x00000001;
    static const int View_SYSTEM_UI_FLAG_HIDE_NAVIGATION = 0x00000002;
    static const int View_SYSTEM_UI_FLAG_FULLSCREEN = 0x00000004;
    static const int View_SYSTEM_UI_FLAG_LAYOUT_STABLE = 0x00000100;
    static const int View_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION = 0x00000200;
    static const int View_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = 0x00000400;
    static const int View_SYSTEM_UI_FLAG_IMMERSIVE_STICKY = 0x00001000;

    const int SDK_INT = app->activity->sdkVersion;
    if (SDK_INT < 11) {
        return;
    }

    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return;
    }

    jobject decorView = _glfmGetDecorView(app);
    if (!decorView) {
        return;
    }
    if (uiChrome == GLFMUserInterfaceChromeNavigationAndStatusBar) {
        _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void, 0);
    } else if (SDK_INT >= 11 && SDK_INT < 14) {
        _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void,
                                    View_STATUS_BAR_HIDDEN);
    } else if (SDK_INT >= 14 && SDK_INT < 19) {
        if (uiChrome == GLFMUserInterfaceChromeNavigation) {
            _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void,
                                        View_SYSTEM_UI_FLAG_FULLSCREEN);
        } else {
            _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void,
                                        View_SYSTEM_UI_FLAG_LOW_PROFILE |
                                        View_SYSTEM_UI_FLAG_FULLSCREEN);
        }
    } else if (SDK_INT >= 19) {
        if (uiChrome == GLFMUserInterfaceChromeNavigation) {
            _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void,
                                        View_SYSTEM_UI_FLAG_FULLSCREEN);
        } else {
            _glfmCallJavaMethodWithArgs(jni, decorView, "setSystemUiVisibility", "(I)V", Void,
                                        View_SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                                        View_SYSTEM_UI_FLAG_FULLSCREEN |
                                        View_SYSTEM_UI_FLAG_LAYOUT_STABLE |
                                        View_SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                                        View_SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                                        View_SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }
    (*jni)->DeleteLocalRef(jni, decorView);
    _glfmClearJavaException()
}

/*
 * Move task to the back if it is root task. This make the back button have the same behavior
 * as the home button.
 *
 * Without this, when the user presses the back button, the loop in android_main() is exited, the
 * OpenGL context is destroyed, and the main thread is destroyed. The android_main() function
 * would be called again in the same process if the user returns to the app.
 *
 * When this, when the app is in the background, the app will pause in the ALooper_pollAll() call.
 */
static bool _glfmHandleBackButton(struct android_app *app) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return false;
    }

    jboolean handled = _glfmCallJavaMethodWithArgs(jni, app->activity->clazz, "moveTaskToBack",
                                                   "(Z)Z", Boolean, false);
    return !_glfmWasJavaExceptionThrown() && handled;
}

static bool _glfmSetKeyboardVisible(GLFMPlatformData *platformData, bool visible) {
    static const int InputMethodManager_SHOW_FORCED = 2;

    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return false;
    }

    jobject decorView = _glfmGetDecorView(platformData->app);
    if (!decorView) {
        return false;
    }

    jclass contextClass = (*jni)->FindClass(jni, "android/content/Context");
    if (_glfmWasJavaExceptionThrown()) {
        return false;
    }

    jstring imString = _glfmGetJavaStaticField(jni, contextClass, "INPUT_METHOD_SERVICE",
                                               "Ljava/lang/String;", Object);
    if (!imString || _glfmWasJavaExceptionThrown()) {
        return false;
    }
    jobject ime = _glfmCallJavaMethodWithArgs(jni, platformData->app->activity->clazz,
                                              "getSystemService",
                                              "(Ljava/lang/String;)Ljava/lang/Object;",
                                              Object, imString);
    if (!ime || _glfmWasJavaExceptionThrown()) {
        return false;
    }

    if (visible) {
        _glfmCallJavaMethodWithArgs(jni, ime, "showSoftInput", "(Landroid/view/View;I)Z", Boolean,
                                    decorView, InputMethodManager_SHOW_FORCED);
    } else {
        jobject windowToken = _glfmCallJavaMethod(jni, decorView, "getWindowToken",
                                                  "()Landroid/os/IBinder;", Object);
        if (!windowToken || _glfmWasJavaExceptionThrown()) {
            return false;
        }
        _glfmCallJavaMethodWithArgs(jni, ime, "hideSoftInputFromWindow",
                                    "(Landroid/os/IBinder;I)Z", Boolean, windowToken, 0);
        (*jni)->DeleteLocalRef(jni, windowToken);
    }

    (*jni)->DeleteLocalRef(jni, ime);
    (*jni)->DeleteLocalRef(jni, imString);
    (*jni)->DeleteLocalRef(jni, contextClass);
    (*jni)->DeleteLocalRef(jni, decorView);

    return !_glfmWasJavaExceptionThrown();
}

static void _glfmResetContentRect(GLFMPlatformData *platformData) {
    // Reset's NativeActivity's content rect so that onContentRectChanged acts as a
    // OnGlobalLayoutListener. This is needed to detect changes to getWindowVisibleDisplayFrame()
    // HACK: This uses undocumented fields.

    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return;
    }

    jfieldID field = _glfmGetJavaFieldID(jni, platformData->app->activity->clazz,
                                         "mLastContentWidth", "I");
    if (!field || _glfmWasJavaExceptionThrown()) {
        return;
    }

    (*jni)->SetIntField(jni, platformData->app->activity->clazz, field, -1);
    _glfmClearJavaException()
}

static ARect _glfmGetWindowVisibleDisplayFrame(GLFMPlatformData *platformData, ARect defaultRect) {
    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return defaultRect;
    }

    jobject decorView = _glfmGetDecorView(platformData->app);
    if (!decorView) {
        return defaultRect;
    }

    jclass javaRectClass = (*jni)->FindClass(jni, "android/graphics/Rect");
    if (_glfmWasJavaExceptionThrown()) {
        return defaultRect;
    }

    jobject javaRect = (*jni)->AllocObject(jni, javaRectClass);
    if (_glfmWasJavaExceptionThrown()) {
        return defaultRect;
    }

    _glfmCallJavaMethodWithArgs(jni, decorView, "getWindowVisibleDisplayFrame",
                                "(Landroid/graphics/Rect;)V", Void, javaRect);
    if (_glfmWasJavaExceptionThrown()) {
        return defaultRect;
    }

    ARect rect;
    rect.left = _glfmGetJavaField(jni, javaRect, "left", "I", Int);
    rect.right = _glfmGetJavaField(jni, javaRect, "right", "I", Int);
    rect.top = _glfmGetJavaField(jni, javaRect, "top", "I", Int);
    rect.bottom = _glfmGetJavaField(jni, javaRect, "bottom", "I", Int);

    (*jni)->DeleteLocalRef(jni, javaRect);
    (*jni)->DeleteLocalRef(jni, javaRectClass);
    (*jni)->DeleteLocalRef(jni, decorView);

    if (_glfmWasJavaExceptionThrown()) {
        return defaultRect;
    } else {
        return rect;
    }
}

static uint32_t _glfmGetUnicodeChar(GLFMPlatformData *platformData, AInputEvent *event) {
    JNIEnv *jni = platformData->jniEnv;
    if ((*jni)->ExceptionCheck(jni)) {
        return 0;
    }

    jint keyCode = AKeyEvent_getKeyCode(event);
    jint metaState = AKeyEvent_getMetaState(event);

    jclass keyEventClass = (*jni)->FindClass(jni, "android/view/KeyEvent");
    if (!keyEventClass || _glfmWasJavaExceptionThrown()) {
        return 0;
    }

    jmethodID getUnicodeChar = (*jni)->GetMethodID(jni, keyEventClass, "getUnicodeChar", "(I)I");
    jmethodID eventConstructor = (*jni)->GetMethodID(jni, keyEventClass, "<init>", "(II)V");

    jobject eventObject = (*jni)->NewObject(jni, keyEventClass, eventConstructor,
                                            AKEY_EVENT_ACTION_DOWN, keyCode);
    if (!keyEventClass || _glfmWasJavaExceptionThrown()) {
        return 0;
    }

    jint unicodeKey = (*jni)->CallIntMethod(jni, eventObject, getUnicodeChar, metaState);

    (*jni)->DeleteLocalRef(jni, eventObject);
    (*jni)->DeleteLocalRef(jni, keyEventClass);

    if (_glfmWasJavaExceptionThrown()) {
        return 0;
    } else {
        return (uint32_t)unicodeKey;
    }
}

// MARK: EGL

static bool _glfmEGLContextInit(GLFMPlatformData *platformData) {

    // Available in eglext.h in API 18
    static const int EGL_CONTEXT_MAJOR_VERSION_KHR = 0x3098;
    static const int EGL_CONTEXT_MINOR_VERSION_KHR = 0x30FB;

    bool created = false;
    if (platformData->eglContext == EGL_NO_CONTEXT) {
        // OpenGL ES 3.2
        if (platformData->display->preferredAPI >= GLFMRenderingAPIOpenGLES32) {
            // TODO: Untested, need an OpenGL ES 3.2 device for testing
            const EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
                                             EGL_CONTEXT_MINOR_VERSION_KHR, 2, EGL_NONE};
            platformData->eglContext = eglCreateContext(platformData->eglDisplay,
                                                        platformData->eglConfig,
                                                        EGL_NO_CONTEXT, contextAttribs);
            created = platformData->eglContext != EGL_NO_CONTEXT;
        }
        // OpenGL ES 3.1
        if (platformData->display->preferredAPI >= GLFMRenderingAPIOpenGLES31) {
            // TODO: Untested, need an OpenGL ES 3.1 device for testing
            const EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
                                             EGL_CONTEXT_MINOR_VERSION_KHR, 1, EGL_NONE};
            platformData->eglContext = eglCreateContext(platformData->eglDisplay,
                                                        platformData->eglConfig,
                                                        EGL_NO_CONTEXT, contextAttribs);
            created = platformData->eglContext != EGL_NO_CONTEXT;
        }
        // OpenGL ES 3.0
        if (!created && platformData->display->preferredAPI >= GLFMRenderingAPIOpenGLES3) {
            const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE, EGL_NONE};
            platformData->eglContext = eglCreateContext(platformData->eglDisplay,
                                                        platformData->eglConfig,
                                                        EGL_NO_CONTEXT, contextAttribs);
            created = platformData->eglContext != EGL_NO_CONTEXT;
        }
        // OpenGL ES 2.0
        if (!created) {
            const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
            platformData->eglContext = eglCreateContext(platformData->eglDisplay,
                                                        platformData->eglConfig,
                                                        EGL_NO_CONTEXT, contextAttribs);
            created = platformData->eglContext != EGL_NO_CONTEXT;
        }

        if (created) {
            EGLint majorVersion = 0;
            EGLint minorVersion = 0;
            eglQueryContext(platformData->eglDisplay, platformData->eglContext,
                            EGL_CONTEXT_MAJOR_VERSION_KHR, &majorVersion);
            if (majorVersion >= 3) { 
                eglQueryContext(platformData->eglDisplay, platformData->eglContext,
                                EGL_CONTEXT_MINOR_VERSION_KHR, &minorVersion);
            }
            if (majorVersion == 3 && minorVersion == 1) {
                platformData->renderingAPI = GLFMRenderingAPIOpenGLES31;
            } else if (majorVersion == 3) {
                platformData->renderingAPI = GLFMRenderingAPIOpenGLES3;
            } else {
                platformData->renderingAPI = GLFMRenderingAPIOpenGLES2;
            }
        }
    }

    if (!eglMakeCurrent(platformData->eglDisplay, platformData->eglSurface,
                        platformData->eglSurface, platformData->eglContext)) {
        LOG_LIFECYCLE("eglMakeCurrent() failed");
        platformData->eglContextCurrent = false;
        return false;
    } else {
        platformData->eglContextCurrent = true;
        if (created && platformData->display) {
            LOG_LIFECYCLE("GL Context made current");
            if (platformData->display->surfaceCreatedFunc) {
                platformData->display->surfaceCreatedFunc(platformData->display,
                                                          platformData->width,
                                                          platformData->height);
            }
        }
        return true;
    }
}

static void _glfmEGLContextDisable(GLFMPlatformData *platformData) {
    if (platformData->eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(platformData->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    platformData->eglContextCurrent = false;
}

static void _glfmEGLSurfaceInit(GLFMPlatformData *platformData) {
    if (platformData->eglSurface == EGL_NO_SURFACE) {
        platformData->eglSurface = eglCreateWindowSurface(platformData->eglDisplay,
                                                          platformData->eglConfig,
                                                          platformData->app->window, NULL);
    }
}

static void _glfmEGLLogConfig(GLFMPlatformData *platformData, EGLConfig config) {
    LOG_DEBUG("Config: %p", config);
    EGLint value;
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_RENDERABLE_TYPE, &value);
    LOG_DEBUG("  EGL_RENDERABLE_TYPE %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_SURFACE_TYPE, &value);
    LOG_DEBUG("  EGL_SURFACE_TYPE    %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_RED_SIZE, &value);
    LOG_DEBUG("  EGL_RED_SIZE        %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_GREEN_SIZE, &value);
    LOG_DEBUG("  EGL_GREEN_SIZE      %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_BLUE_SIZE, &value);
    LOG_DEBUG("  EGL_BLUE_SIZE       %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_ALPHA_SIZE, &value);
    LOG_DEBUG("  EGL_ALPHA_SIZE      %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_DEPTH_SIZE, &value);
    LOG_DEBUG("  EGL_DEPTH_SIZE      %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_STENCIL_SIZE, &value);
    LOG_DEBUG("  EGL_STENCIL_SIZE    %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_SAMPLE_BUFFERS, &value);
    LOG_DEBUG("  EGL_SAMPLE_BUFFERS  %i", value);
    eglGetConfigAttrib(platformData->eglDisplay, config, EGL_SAMPLES, &value);
    LOG_DEBUG("  EGL_SAMPLES         %i", value);
}

static bool _glfmEGLInit(GLFMPlatformData *platformData) {
    if (platformData->eglDisplay != EGL_NO_DISPLAY) {
        _glfmEGLSurfaceInit(platformData);
        return _glfmEGLContextInit(platformData);
    }
    int rBits, gBits, bBits, aBits;
    int depthBits, stencilBits, samples;

    switch (platformData->display->colorFormat) {
        case GLFMColorFormatRGB565:
            rBits = 5;
            gBits = 6;
            bBits = 5;
            aBits = 0;
            break;
        case GLFMColorFormatRGBA8888:
        default:
            rBits = 8;
            gBits = 8;
            bBits = 8;
            aBits = 8;
            break;
    }

    switch (platformData->display->depthFormat) {
        case GLFMDepthFormatNone:
        default:
            depthBits = 0;
            break;
        case GLFMDepthFormat16:
            depthBits = 16;
            break;
        case GLFMDepthFormat24:
            depthBits = 24;
            break;
    }

    switch (platformData->display->stencilFormat) {
        case GLFMStencilFormatNone:
        default:
            stencilBits = 0;
            break;
        case GLFMStencilFormat8:
            stencilBits = 8;
            if (depthBits > 0) {
                // Many implementations only allow 24-bit depth with 8-bit stencil.
                depthBits = 24;
            }
            break;
    }

    samples = platformData->display->multisample == GLFMMultisample4X ? 4 : 0;

    EGLint majorVersion;
    EGLint minorVersion;
    EGLint format;
    EGLint numConfigs;

    platformData->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(platformData->eglDisplay, &majorVersion, &minorVersion);

    while (true) {
        const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, rBits,
            EGL_GREEN_SIZE, gBits,
            EGL_BLUE_SIZE, bBits,
            EGL_ALPHA_SIZE, aBits,
            EGL_DEPTH_SIZE, depthBits,
            EGL_STENCIL_SIZE, stencilBits,
            EGL_SAMPLE_BUFFERS, samples > 0 ? 1 : 0,
            EGL_SAMPLES, samples > 0 ? samples : 0,
            EGL_NONE};

        eglChooseConfig(platformData->eglDisplay, attribs, &platformData->eglConfig, 1, &numConfigs);
        if (numConfigs) {
            // Found!
            //_glfmEGLLogConfig(platformData, platformData->eglConfig);
            break;
        } else if (samples > 0) {
            // Try 2x multisampling or no multisampling
            samples -= 2;
        } else if (depthBits > 8) {
            // Try 16-bit depth or 8-bit depth
            depthBits -= 8;
        } else {
            // Failure
            static bool printedConfigs = false;
            if (!printedConfigs) {
                printedConfigs = true;
                LOG_DEBUG("eglChooseConfig() failed");
                EGLConfig configs[256];
                EGLint numTotalConfigs;
                if (eglGetConfigs(platformData->eglDisplay, configs, 256, &numTotalConfigs)) {
                    LOG_DEBUG("Num available configs: %i", numTotalConfigs);
                    int i;
                    for (i = 0; i < numTotalConfigs; i++) {
                        _glfmEGLLogConfig(platformData, configs[i]);
                    }
                } else {
                    LOG_DEBUG("Couldn't get any EGL configs");
                }
            }

            _glfmReportSurfaceError(platformData->eglDisplay, "eglChooseConfig() failed");
            eglTerminate(platformData->eglDisplay);
            platformData->eglDisplay = EGL_NO_DISPLAY;
            return false;
        }
    }

    _glfmEGLSurfaceInit(platformData);

    eglQuerySurface(platformData->eglDisplay, platformData->eglSurface, EGL_WIDTH,
                    &platformData->width);
    eglQuerySurface(platformData->eglDisplay, platformData->eglSurface, EGL_HEIGHT,
                    &platformData->height);
    eglGetConfigAttrib(platformData->eglDisplay, platformData->eglConfig, EGL_NATIVE_VISUAL_ID,
                       &format);

    ANativeWindow_setBuffersGeometry(platformData->app->window, 0, 0, format);

    return _glfmEGLContextInit(platformData);
}

static void _glfmEGLSurfaceDestroy(GLFMPlatformData *platformData) {
    if (platformData->eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(platformData->eglDisplay, platformData->eglSurface);
        platformData->eglSurface = EGL_NO_SURFACE;
    }
    _glfmEGLContextDisable(platformData);
}

static void _glfmEGLDestroy(GLFMPlatformData *platformData) {
    if (platformData->eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(platformData->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (platformData->eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(platformData->eglDisplay, platformData->eglContext);
            if (platformData->display) {
                LOG_LIFECYCLE("GL Context destroyed");
                if (platformData->display->surfaceDestroyedFunc) {
                    platformData->display->surfaceDestroyedFunc(platformData->display);
                }
            }
        }
        if (platformData->eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(platformData->eglDisplay, platformData->eglSurface);
        }
        eglTerminate(platformData->eglDisplay);
    }
    platformData->eglDisplay = EGL_NO_DISPLAY;
    platformData->eglContext = EGL_NO_CONTEXT;
    platformData->eglSurface = EGL_NO_SURFACE;
    platformData->eglContextCurrent = false;
}

static void _glfmEGLCheckError(GLFMPlatformData *platformData) {
    EGLint err = eglGetError();
    if (err == EGL_BAD_SURFACE) {
        _glfmEGLSurfaceDestroy(platformData);
        _glfmEGLSurfaceInit(platformData);
    } else if (err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT) {
        if (platformData->eglContext != EGL_NO_CONTEXT) {
            platformData->eglContext = EGL_NO_CONTEXT;
            platformData->eglContextCurrent = false;
            if (platformData->display) {
                LOG_LIFECYCLE("GL Context lost");
                if (platformData->display->surfaceDestroyedFunc) {
                    platformData->display->surfaceDestroyedFunc(platformData->display);
                }
            }
        }
        _glfmEGLContextInit(platformData);
    } else {
        _glfmEGLDestroy(platformData);
        _glfmEGLInit(platformData);
    }
}

static void _glfmDrawFrame(GLFMPlatformData *platformData) {
    if (!platformData->eglContextCurrent) {
        // Probably a bad config (Happens on Android 2.3 emulator)
        return;
    }

    // Check for resize (or rotate)
    int32_t width;
    int32_t height;
    eglQuerySurface(platformData->eglDisplay, platformData->eglSurface, EGL_WIDTH, &width);
    eglQuerySurface(platformData->eglDisplay, platformData->eglSurface, EGL_HEIGHT, &height);
    if (width != platformData->width || height != platformData->height) {
        LOG_LIFECYCLE("Resize: %i x %i", width, height);
        platformData->width = width;
        platformData->height = height;
        if (platformData->display && platformData->display->surfaceResizedFunc) {
            platformData->display->surfaceResizedFunc(platformData->display, width, height);
        }
    }

    // Tick and draw
    if (platformData->display && platformData->display->mainLoopFunc) {
        const double frameTime = _glfmTimeSeconds(
                _glfmTimeSubstract(_glfmTimeNow(), platformData->initTime));
        platformData->display->mainLoopFunc(platformData->display, frameTime);
    } else {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // Swap
    if (!eglSwapBuffers(platformData->eglDisplay, platformData->eglSurface)) {
        _glfmEGLCheckError(platformData);
    }
}

// MARK: Native app glue extension

static bool ARectsEqual(ARect r1, ARect r2) {
    return r1.left == r2.left && r1.top == r2.top && r1.right == r2.right && r1.bottom == r2.bottom;
}

static void _glfmWriteCmd(struct android_app *android_app, int8_t cmd) {
    write(android_app->msgwrite, &cmd, sizeof(cmd));
}

static void _glfmSetContentRect(struct android_app *android_app, ARect rect) {
    pthread_mutex_lock(&android_app->mutex);
    android_app->pendingContentRect = rect;
    _glfmWriteCmd(android_app, APP_CMD_CONTENT_RECT_CHANGED);
    while (!ARectsEqual(android_app->contentRect, android_app->pendingContentRect)) {
        pthread_cond_wait(&android_app->cond, &android_app->mutex);
    }
    pthread_mutex_unlock(&android_app->mutex);
}

static void _glfmOnContentRectChanged(ANativeActivity *activity, const ARect *rect) {
    _glfmSetContentRect((struct android_app *)activity->instance, *rect);
}

// MARK: Keyboard visibility

static void _glfmUpdateKeyboardVisibility(GLFMPlatformData *platformData) {
    if (platformData->display) {
        ARect windowRect = platformData->app->contentRect;
        ARect visibleRect = _glfmGetWindowVisibleDisplayFrame(platformData, windowRect);
        ARect nonVisibleRect[4];

        // Left
        nonVisibleRect[0].left = windowRect.left;
        nonVisibleRect[0].right = visibleRect.left;
        nonVisibleRect[0].top = windowRect.top;
        nonVisibleRect[0].bottom = windowRect.bottom;

        // Right
        nonVisibleRect[1].left = visibleRect.right;
        nonVisibleRect[1].right = windowRect.right;
        nonVisibleRect[1].top = windowRect.top;
        nonVisibleRect[1].bottom = windowRect.bottom;

        // Top
        nonVisibleRect[2].left = windowRect.left;
        nonVisibleRect[2].right = windowRect.right;
        nonVisibleRect[2].top = windowRect.top;
        nonVisibleRect[2].bottom = visibleRect.top;

        // Bottom
        nonVisibleRect[3].left = windowRect.left;
        nonVisibleRect[3].right = windowRect.right;
        nonVisibleRect[3].top = visibleRect.bottom;
        nonVisibleRect[3].bottom = windowRect.bottom;

        // Find largest with minimum keyboard size
        const int minimumKeyboardSize = (int)(100 * platformData->scale);
        int largestIndex = 0;
        int largestArea = -1;
        for (int i = 0; i < 4; i++) {
            int w = nonVisibleRect[i].right - nonVisibleRect[i].left;
            int h = nonVisibleRect[i].bottom - nonVisibleRect[i].top;
            int area = w * h;
            if (w >= minimumKeyboardSize && h >= minimumKeyboardSize && area > largestArea) {
                largestIndex = i;
                largestArea = area;
            }
        }

        bool keyboardVisible = largestArea > 0;
        ARect keyboardFrame = keyboardVisible ? nonVisibleRect[largestIndex] : (ARect){0, 0, 0, 0};

        // Send update notification
        if (platformData->keyboardVisible != keyboardVisible ||
                !ARectsEqual(platformData->keyboardFrame, keyboardFrame)) {
            platformData->keyboardVisible = keyboardVisible;
            platformData->keyboardFrame = keyboardFrame;
            if (platformData->display->keyboardVisibilityChangedFunc) {
                double x = keyboardFrame.left;
                double y = keyboardFrame.top;
                double w = keyboardFrame.right - keyboardFrame.left;
                double h = keyboardFrame.bottom - keyboardFrame.top;
                platformData->display->keyboardVisibilityChangedFunc(platformData->display,
                                                                     keyboardVisible,
                                                                     x, y, w, h);
            }
        }
    }
}

// MARK: App command callback

static void _glfmSetAnimating(GLFMPlatformData *platformData, bool animating) {
    if (platformData->animating != animating) {
        bool sendAppEvent = true;
        if (!platformData->hasInited && animating) {
            platformData->hasInited = true;
            platformData->initTime = _glfmTimeNow();
            sendAppEvent = false;
        }
        platformData->animating = animating;
        if (sendAppEvent && platformData->display && platformData->display->focusFunc) {
            platformData->display->focusFunc(platformData->display, animating);
        }
    }
}

static void _glfmOnAppCmd(struct android_app *app, int32_t cmd) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE: {
            LOG_LIFECYCLE("APP_CMD_SAVE_STATE");
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            LOG_LIFECYCLE("APP_CMD_INIT_WINDOW");
            const bool success = _glfmEGLInit(platformData);
            if (!success) {
                _glfmEGLCheckError(platformData);
            }
            _glfmDrawFrame(platformData);
            break;
        }
        case APP_CMD_WINDOW_RESIZED: {
            LOG_LIFECYCLE("APP_CMD_WINDOW_RESIZED");
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            LOG_LIFECYCLE("APP_CMD_TERM_WINDOW");
            _glfmEGLSurfaceDestroy(platformData);
            _glfmSetAnimating(platformData, false);
            break;
        }
        case APP_CMD_WINDOW_REDRAW_NEEDED: {
            LOG_LIFECYCLE("APP_CMD_WINDOW_REDRAW_NEEDED");
            _glfmDrawFrame(platformData);
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            LOG_LIFECYCLE("APP_CMD_GAINED_FOCUS");
            _glfmSetAnimating(platformData, true);
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            LOG_LIFECYCLE("APP_CMD_LOST_FOCUS");
            if (platformData->animating) {
                _glfmDrawFrame(platformData);
                _glfmSetAnimating(platformData, false);
            }
            break;
        }
        case APP_CMD_CONTENT_RECT_CHANGED: {
            LOG_LIFECYCLE("APP_CMD_CONTENT_RECT_CHANGED");
            pthread_mutex_lock(&app->mutex);
            app->contentRect = app->pendingContentRect;
            _glfmResetContentRect(platformData);
            pthread_cond_broadcast(&app->cond);
            pthread_mutex_unlock(&app->mutex);
            _glfmUpdateKeyboardVisibility(platformData);
            break;
        }
        case APP_CMD_LOW_MEMORY: {
            LOG_LIFECYCLE("APP_CMD_LOW_MEMORY");
            if (platformData->display && platformData->display->lowMemoryFunc) {
                platformData->display->lowMemoryFunc(platformData->display);
            }
            break;
        }
        case APP_CMD_START: {
            LOG_LIFECYCLE("APP_CMD_START");
            _glfmSetFullScreen(app, platformData->display->uiChrome);
            break;
        }
        case APP_CMD_RESUME: {
            LOG_LIFECYCLE("APP_CMD_RESUME");
            break;
        }
        case APP_CMD_PAUSE: {
            LOG_LIFECYCLE("APP_CMD_PAUSE");
            break;
        }
        case APP_CMD_STOP: {
            LOG_LIFECYCLE("APP_CMD_STOP");
            break;
        }
        case APP_CMD_DESTROY: {
            LOG_LIFECYCLE("APP_CMD_DESTROY");
            _glfmEGLDestroy(platformData);
            break;
        }
        default: {
            // Do nothing
            break;
        }
    }
}

// MARK: Key and touch input callback

static const char *_glfmUnicodeToUTF8(uint32_t unicode) {
    static char utf8[5];
    if (unicode < 0x80) {
        utf8[0] = (char)(unicode & 0x7f);
        utf8[1] = 0;
    } else if (unicode < 0x800) {
        utf8[0] = (char)(0xc0 | (unicode >> 6));
        utf8[1] = (char)(0x80 | (unicode & 0x3f));
        utf8[2] = 0;
    } else if (unicode < 0x10000) {
        utf8[0] = (char)(0xe0 | (unicode >> 12));
        utf8[1] = (char)(0x80 | ((unicode >> 6) & 0x3f));
        utf8[2] = (char)(0x80 | (unicode & 0x3f));
        utf8[3] = 0;
    } else if (unicode < 0x110000) {
        utf8[0] = (char)(0xf0 | (unicode >> 18));
        utf8[1] = (char)(0x80 | ((unicode >> 12) & 0x3f));
        utf8[2] = (char)(0x80 | ((unicode >> 6) & 0x3f));
        utf8[3] = (char)(0x80 | (unicode & 0x3f));
        utf8[4] = 0;
    } else {
        utf8[0] = 0;
    }
    return utf8;
}

static int32_t _glfmOnInputEvent(struct android_app *app, AInputEvent *event) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)app->userData;
    const int32_t eventType = AInputEvent_getType(event);
    if (eventType == AINPUT_EVENT_TYPE_KEY) {
        int handled = 0;
        if (platformData->display && platformData->display->keyFunc) {
            int32_t aKeyCode = AKeyEvent_getKeyCode(event);
            int32_t aAction = AKeyEvent_getAction(event);
            if (aKeyCode != 0) {
                GLFMKey key;
                switch (aKeyCode) {
                    case AKEYCODE_DPAD_LEFT:
                        key = GLFMKeyLeft;
                        break;
                    case AKEYCODE_DPAD_RIGHT:
                        key = GLFMKeyRight;
                        break;
                    case AKEYCODE_DPAD_UP:
                        key = GLFMKeyUp;
                        break;
                    case AKEYCODE_DPAD_DOWN:
                        key = GLFMKeyDown;
                        break;
                    case AKEYCODE_ENTER:
                    case AKEYCODE_DPAD_CENTER:
                        key = GLFMKeyEnter;
                        break;
                    case AKEYCODE_TAB:
                        key = GLFMKeyTab;
                        break;
                    case AKEYCODE_SPACE:
                        key = GLFMKeySpace;
                        break;
                    case AKEYCODE_BACK:
                        key = GLFMKeyNavBack;
                        break;
                    case AKEYCODE_MENU:
                        key = GLFMKeyNavMenu;
                        break;
                    default:
                        // TODO: Send all keycodes?
                        if (aKeyCode >= AKEYCODE_0 && aKeyCode <= AKEYCODE_9) {
                            key = (GLFMKey)(aKeyCode - AKEYCODE_0 + '0');
                        } else if (aKeyCode >= AKEYCODE_A && aKeyCode <= AKEYCODE_Z) {
                            key = (GLFMKey)(aKeyCode - AKEYCODE_A + 'A');
                        } else {
                            key = (GLFMKey)0;
                        }
                        break;
                }

                if (key != 0) {
                    if (aAction == AKEY_EVENT_ACTION_UP) {
                        handled = platformData->display->keyFunc(platformData->display, key,
                                                                 GLFMKeyActionReleased, 0);
                        if (handled == 0 && aKeyCode == AKEYCODE_BACK) {
                            handled = _glfmHandleBackButton(app) ? 1 : 0;
                        }
                    } else if (aAction == AKEY_EVENT_ACTION_DOWN) {
                        GLFMKeyAction keyAction;
                        if (AKeyEvent_getRepeatCount(event) > 0) {
                            keyAction = GLFMKeyActionRepeated;
                        } else {
                            keyAction = GLFMKeyActionPressed;
                        }
                        handled = platformData->display->keyFunc(platformData->display, key,
                                                                 keyAction, 0);
                    } else if (aAction == AKEY_EVENT_ACTION_MULTIPLE) {
                        int32_t i;
                        for (i = AKeyEvent_getRepeatCount(event); i > 0; i--) {
                            handled |= platformData->display->keyFunc(platformData->display, key,
                                                                GLFMKeyActionPressed, 0);
                            handled |= platformData->display->keyFunc(platformData->display, key,
                                                                GLFMKeyActionReleased, 0);
                        }
                    }
                }
            }
        }
        if (platformData->display && platformData->display->charFunc) {
            int32_t aAction = AKeyEvent_getAction(event);
            if (aAction == AKEY_EVENT_ACTION_DOWN || aAction == AKEY_EVENT_ACTION_MULTIPLE) {
                uint32_t unicode = _glfmGetUnicodeChar(platformData, event);
                if (unicode >= ' ') {
                    const char *str = _glfmUnicodeToUTF8(unicode);
                    if (aAction == AKEY_EVENT_ACTION_DOWN) {
                        platformData->display->charFunc(platformData->display, str, 0);
                    } else {
                        int32_t i;
                        for (i = AKeyEvent_getRepeatCount(event); i > 0; i--) {
                            platformData->display->charFunc(platformData->display, str, 0);
                        }
                    }
                }
            }
        }
        return handled;
    } else if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        if (platformData->display && platformData->display->touchFunc) {
            const int maxTouches = platformData->multitouchEnabled ? MAX_SIMULTANEOUS_TOUCHES : 1;
            const int32_t action = AMotionEvent_getAction(event);
            const int maskedAction = action & AMOTION_EVENT_ACTION_MASK;

            GLFMTouchPhase phase;
            bool validAction = true;

            switch (maskedAction) {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                    phase = GLFMTouchPhaseBegan;
                    break;
                case AMOTION_EVENT_ACTION_UP:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                case AMOTION_EVENT_ACTION_OUTSIDE:
                    phase = GLFMTouchPhaseEnded;
                    break;
                case AMOTION_EVENT_ACTION_MOVE:
                    phase = GLFMTouchPhaseMoved;
                    break;
                case AMOTION_EVENT_ACTION_CANCEL:
                    phase = GLFMTouchPhaseCancelled;
                    break;
                default:
                    phase = GLFMTouchPhaseCancelled;
                    validAction = false;
                    break;
            }
            if (validAction) {
                if (phase == GLFMTouchPhaseMoved) {
                    const size_t count = AMotionEvent_getPointerCount(event);
                    size_t i;
                    for (i = 0; i < count; i++) {
                        const int touchNumber = AMotionEvent_getPointerId(event, i);
                        if (touchNumber >= 0 && touchNumber < maxTouches) {
                            double x = (double)AMotionEvent_getX(event, i);
                            double y = (double)AMotionEvent_getY(event, i);
                            platformData->display->touchFunc(platformData->display, touchNumber,
                                                             phase, x, y);
                            //LOG_DEBUG("Touch %i: (%i) %i,%i", touchNumber, phase, x, y);
                        }
                    }
                } else {
                    const size_t index =
                        (size_t)((action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                 AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
                    const int touchNumber = AMotionEvent_getPointerId(event, index);
                    if (touchNumber >= 0 && touchNumber < maxTouches) {
                        double x = (double)AMotionEvent_getX(event, index);
                        double y = (double)AMotionEvent_getY(event, index);
                        platformData->display->touchFunc(platformData->display, touchNumber,
                                                         phase, x, y);
                        //LOG_DEBUG("Touch %i: (%i) %i,%i", touchNumber, phase, x, y);
                    }
                }
            }
        }
        return 1;
    }
    return 0;
}

// MARK: Main entry point

void android_main(struct android_app *app) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Don't strip glue code. Although this is deprecated, it's easier with complex CMake files.
    app_dummy();
#pragma clang diagnostic pop

    LOG_LIFECYCLE("android_main");

    // Init platform data
    GLFMPlatformData *platformData;
    if (platformDataGlobal == NULL) {
        platformDataGlobal = calloc(1, sizeof(GLFMPlatformData));
    }
    platformData = platformDataGlobal;

    app->userData = platformData;
    app->onAppCmd = _glfmOnAppCmd;
    app->onInputEvent = _glfmOnInputEvent;
    app->activity->callbacks->onContentRectChanged = _glfmOnContentRectChanged;
    platformData->app = app;

    // Init java env
    JavaVM *vm = app->activity->vm;
    (*vm)->AttachCurrentThread(vm, &platformData->jniEnv, NULL);

    // Get display scale
    const int ACONFIGURATION_DENSITY_ANY = 0xfffe; // Added in API 21
    const int32_t density = AConfiguration_getDensity(app->config);
    if (density == ACONFIGURATION_DENSITY_DEFAULT || density == ACONFIGURATION_DENSITY_NONE ||
            density == ACONFIGURATION_DENSITY_ANY || density <= 0) {
        platformData->scale = 1.0;
    } else {
        platformData->scale = density / 160.0;
    }

    if (platformData->display == NULL) {
        LOG_LIFECYCLE("glfmMain");
        // Only call glfmMain() once per instance
        // This should call glfmInit()
        platformData->display = calloc(1, sizeof(GLFMDisplay));
        platformData->display->platformData = platformData;
        glfmMain(platformData->display);
    }

    // Setup window params
    int32_t windowFormat;
    switch (platformData->display->colorFormat) {
        case GLFMColorFormatRGB565:
            windowFormat = WINDOW_FORMAT_RGB_565;
            break;
        case GLFMColorFormatRGBA8888: default:
            windowFormat = WINDOW_FORMAT_RGBA_8888;
            break;
    }
    bool fullscreen = platformData->display->uiChrome == GLFMUserInterfaceChromeFullscreen;
    ANativeActivity_setWindowFormat(app->activity, windowFormat);
    ANativeActivity_setWindowFlags(app->activity,
                                   fullscreen ? AWINDOW_FLAG_FULLSCREEN : 0,
                                   AWINDOW_FLAG_FULLSCREEN);
    _glfmSetFullScreen(app, platformData->display->uiChrome);

    // Run the main loop
    while (1) {
        int ident;
        int events;
        struct android_poll_source *source;

        while ((ident = ALooper_pollAll(platformData->animating ? 0 : -1, NULL, &events,
                                        (void **)&source)) >= 0) {
            if (source) {
                source->process(app, source);
            }

//          if (ident == LOOPER_ID_USER) {
//              if (platformData->accelerometerSensor != NULL) {
//                  ASensorEvent event;
//                  while (ASensorEventQueue_getEvents(platformData->sensorEventQueue,
//                                                     &event, 1) > 0) {
//                      LOG_DEBUG("accelerometer: x=%f y=%f z=%f",
//                           event.acceleration.x, event.acceleration.y,
//                           event.acceleration.z);
//                  }
//              }
//          }

            if (app->destroyRequested != 0) {
                LOG_LIFECYCLE("Destroying thread");
                _glfmEGLDestroy(platformData);
                _glfmSetAnimating(platformData, false);
                (*vm)->DetachCurrentThread(vm);
                platformData->app = NULL;
                // App is destroyed, but android_main() can be called again in the same process.
                return;
            }
        }

        if (platformData->animating && platformData->display) {
            _glfmDrawFrame(platformData);
        }
    }
}

// MARK: GLFM implementation

void glfmSetUserInterfaceOrientation(GLFMDisplay *display,
                                     GLFMUserInterfaceOrientation allowedOrientations) {
    if (display->allowedOrientations != allowedOrientations) {
        display->allowedOrientations = allowedOrientations;
        GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
        _glfmSetOrientation(platformData->app);
    }
}

void glfmGetDisplaySize(GLFMDisplay *display, int *width, int *height) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    *width = platformData->width;
    *height = platformData->height;
}

double glfmGetDisplayScale(GLFMDisplay *display) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    return platformData->scale;
}

void glfmGetDisplayChromeInsets(GLFMDisplay *display, double *top, double *right, double *bottom,
                                double *left) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    ARect windowRect = platformData->app->contentRect;
    ARect visibleRect = _glfmGetWindowVisibleDisplayFrame(platformData, windowRect);
    if (visibleRect.right - visibleRect.left <= 0 || visibleRect.bottom - visibleRect.top <= 0) {
        *top = 0;
        *right = 0;
        *bottom = 0;
        *left = 0;
    } else {
        *top = visibleRect.top;
        *right = platformData->width - visibleRect.right;
        *bottom = platformData->height - visibleRect.bottom;
        *left = visibleRect.left;
    }
}

void _glfmDisplayChromeUpdated(GLFMDisplay *display) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    _glfmSetFullScreen(platformData->app, display->uiChrome);
}

GLFMRenderingAPI glfmGetRenderingAPI(GLFMDisplay *display) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    return platformData->renderingAPI;
}

bool glfmHasTouch(GLFMDisplay *display) {
    (void)display;
    // This will need to change, for say, TV apps
    return true;
}

void glfmSetMouseCursor(GLFMDisplay *display, GLFMMouseCursor mouseCursor) {
    (void)display;
    (void)mouseCursor;
    // Do nothing
}

void glfmSetMultitouchEnabled(GLFMDisplay *display, bool multitouchEnabled) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    platformData->multitouchEnabled = multitouchEnabled;
}

bool glfmGetMultitouchEnabled(GLFMDisplay *display) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    return platformData->multitouchEnabled;
}

GLFMProc glfmGetProcAddress(const char *functionName) {
    GLFMProc function = eglGetProcAddress(functionName);
    if (!function) {
        static void *handle = NULL;
        if (!handle) {
            handle = dlopen(NULL, RTLD_LAZY);
        }
        function = handle ? (GLFMProc)dlsym(handle, functionName) : NULL;
    }
    return function;
}

void glfmSetKeyboardVisible(GLFMDisplay *display, bool visible) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    if (_glfmSetKeyboardVisible(platformData, visible)) {
        if (visible && display->uiChrome == GLFMUserInterfaceChromeFullscreen) {
            // This seems to be required to reset to fullscreen when the keyboard is shown.
            _glfmSetFullScreen(platformData->app, GLFMUserInterfaceChromeNavigationAndStatusBar);
        }
    }
}

bool glfmIsKeyboardVisible(GLFMDisplay *display) {
    GLFMPlatformData *platformData = (GLFMPlatformData *)display->platformData;
    return platformData->keyboardVisible;
}

// MARK: Android-specific public functions

ANativeActivity *glfmAndroidGetActivity() {
    if (platformDataGlobal && platformDataGlobal->app) {
        return platformDataGlobal->app->activity;
    } else {
        return NULL;
    }
}

#endif
