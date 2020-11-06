#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QPainter>

#include "sceneview.h"

#include <engine.h>
#include <system.h>

#include <components/actor.h>
#include <components/scene.h>
#include <components/camera.h>

#include <resources/pipeline.h>

#include "controllers/cameractrl.h"

#include "pluginmanager.h"

#define NONE 0
#define RELEASE 1
#define PRESS 2
#define REPEAT 3

SceneView::SceneView(QWidget *parent) :
        QOpenGLWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_pEngine(nullptr) {

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

SceneView::~SceneView() {

}

void SceneView::setEngine(Engine *engine) {
    m_pEngine = engine;
}

void SceneView::setScene(Scene *scene) {
    if(m_pScene) {
        m_pScene->deleteLater();
    }
    m_pScene    = scene;
    PluginManager::instance()->addScene(m_pScene);
}

void SceneView::setController(CameraCtrl *ctrl) {
    m_pController   = ctrl;
}

void SceneView::initializeGL() {
    emit inited();
}

void SceneView::paintGL() {
    if(m_pController) {
        m_pController->update();
    }
    if(m_pScene) {
        findCamera();

        if(m_pEngine) {
            m_pEngine->processEvents();
        }

        PluginManager::instance()->updateSystems(m_pScene);

        if(m_pEngine) {
            for(auto &it : m_Keys) {
                switch(it.second) {
                    case RELEASE: it.second = NONE; break;
                    case PRESS: it.second = REPEAT; break;
                    default: break;
                }
            }

            for(auto &it : m_MouseButtons) {
                switch(it.second) {
                    case RELEASE: it.second = NONE; break;
                    case PRESS: it.second = REPEAT; break;
                    default: break;
                }
            }
        }
    }
}

void SceneView::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);

    if(m_pController) {
        Camera *camera  = Camera::current();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width, height);
            pipe->setTarget(defaultFramebufferObject());
        }
    }
}

Input::KeyCode mapToInput(int32_t key) {
    Input::KeyCode result = Input::KEY_UNKNOWN;

    switch(key) {
    case Qt::Key_Space: result = Input::KEY_SPACE; break;
    case Qt::Key_Apostrophe: result = Input::KEY_APOSTROPHE; break;
    case Qt::Key_Comma: result = Input::KEY_COMMA; break;
    case Qt::Key_Minus: result = Input::KEY_MINUS; break;
    case Qt::Key_Period: result = Input::KEY_PERIOD; break;
    case Qt::Key_Slash: result = Input::KEY_SLASH; break;
    case Qt::Key_0: result = Input::KEY_0; break;
    case Qt::Key_1: result = Input::KEY_1; break;
    case Qt::Key_2: result = Input::KEY_2; break;
    case Qt::Key_3: result = Input::KEY_3; break;
    case Qt::Key_4: result = Input::KEY_4; break;
    case Qt::Key_5: result = Input::KEY_5; break;
    case Qt::Key_6: result = Input::KEY_6; break;
    case Qt::Key_7: result = Input::KEY_7; break;
    case Qt::Key_8: result = Input::KEY_8; break;
    case Qt::Key_9: result = Input::KEY_9; break;
    case Qt::Key_Semicolon: result = Input::KEY_SEMICOLON; break;
    case Qt::Key_Equal: result = Input::KEY_EQUAL; break;
    case Qt::Key_A: result = Input::KEY_A; break;
    case Qt::Key_B: result = Input::KEY_B; break;
    case Qt::Key_C: result = Input::KEY_C; break;
    case Qt::Key_D: result = Input::KEY_D; break;
    case Qt::Key_E: result = Input::KEY_E; break;
    case Qt::Key_F: result = Input::KEY_F; break;
    case Qt::Key_G: result = Input::KEY_G; break;
    case Qt::Key_H: result = Input::KEY_H; break;
    case Qt::Key_I: result = Input::KEY_I; break;
    case Qt::Key_J: result = Input::KEY_J; break;
    case Qt::Key_K: result = Input::KEY_K; break;
    case Qt::Key_L: result = Input::KEY_L; break;
    case Qt::Key_M: result = Input::KEY_M; break;
    case Qt::Key_N: result = Input::KEY_N; break;
    case Qt::Key_O: result = Input::KEY_O; break;
    case Qt::Key_P: result = Input::KEY_P; break;
    case Qt::Key_Q: result = Input::KEY_Q; break;
    case Qt::Key_R: result = Input::KEY_R; break;
    case Qt::Key_S: result = Input::KEY_S; break;
    case Qt::Key_T: result = Input::KEY_T; break;
    case Qt::Key_U: result = Input::KEY_U; break;
    case Qt::Key_V: result = Input::KEY_V; break;
    case Qt::Key_W: result = Input::KEY_W; break;
    case Qt::Key_X: result = Input::KEY_X; break;
    case Qt::Key_Y: result = Input::KEY_Y; break;
    case Qt::Key_Z: result = Input::KEY_Z; break;
    case Qt::Key_BracketLeft: result = Input::KEY_LEFT_BRACKET; break;
    case Qt::Key_Backslash: result = Input::KEY_BACKSLASH; break;
    case Qt::Key_BracketRight: result = Input::KEY_RIGHT_BRACKET; break;
    case Qt::Key_Agrave: result = Input::KEY_GRAVE_ACCENT; break;
    //result = Input::KEY_WORLD_1; break;
    //result = Input::KEY_WORLD_2; break;
    case Qt::Key_Escape: result = Input::KEY_ESCAPE; break;
    case Qt::Key_Enter: result = Input::KEY_ENTER; break;
    case Qt::Key_Tab: result = Input::KEY_TAB; break;
    case Qt::Key_Backspace: result = Input::KEY_BACKSPACE; break;
    case Qt::Key_Insert: result = Input::KEY_INSERT; break;
    case Qt::Key_Delete: result = Input::KEY_DELETE; break;
    case Qt::Key_Right: result = Input::KEY_RIGHT; break;
    case Qt::Key_Left: result = Input::KEY_LEFT; break;
    case Qt::Key_Down: result = Input::KEY_DOWN; break;
    case Qt::Key_Up: result = Input::KEY_UP; break;
    case Qt::Key_PageUp: result = Input::KEY_PAGE_UP; break;
    case Qt::Key_PageDown: result = Input::KEY_PAGE_DOWN; break;
    case Qt::Key_Home: result = Input::KEY_HOME; break;
    case Qt::Key_End: result = Input::KEY_END; break;
    case Qt::Key_CapsLock: result = Input::KEY_CAPS_LOCK; break;
    case Qt::Key_ScrollLock: result = Input::KEY_SCROLL_LOCK; break;
    case Qt::Key_NumLock: result = Input::KEY_NUM_LOCK; break;
    //result = Input::KEY_PRINT_SCREEN; break;
    case Qt::Key_Pause: result = Input::KEY_PAUSE; break;
    case Qt::Key_F1 : result = Input::KEY_F1; break;
    case Qt::Key_F2 : result = Input::KEY_F2; break;
    case Qt::Key_F3 : result = Input::KEY_F3; break;
    case Qt::Key_F4 : result = Input::KEY_F4; break;
    case Qt::Key_F5 : result = Input::KEY_F5; break;
    case Qt::Key_F6 : result = Input::KEY_F6; break;
    case Qt::Key_F7 : result = Input::KEY_F7; break;
    case Qt::Key_F8 : result = Input::KEY_F8; break;
    case Qt::Key_F9 : result = Input::KEY_F9; break;
    case Qt::Key_F10: result = Input::KEY_F10; break;
    case Qt::Key_F11: result = Input::KEY_F11; break;
    case Qt::Key_F12: result = Input::KEY_F12; break;
    case Qt::Key_F13: result = Input::KEY_F13; break;
    case Qt::Key_F14: result = Input::KEY_F14; break;
    case Qt::Key_F15: result = Input::KEY_F15; break;
    case Qt::Key_F16: result = Input::KEY_F16; break;
    case Qt::Key_F17: result = Input::KEY_F17; break;
    case Qt::Key_F18: result = Input::KEY_F18; break;
    case Qt::Key_F19: result = Input::KEY_F19; break;
    case Qt::Key_F20: result = Input::KEY_F20; break;
    case Qt::Key_F21: result = Input::KEY_F21; break;
    case Qt::Key_F22: result = Input::KEY_F22; break;
    case Qt::Key_F23: result = Input::KEY_F23; break;
    case Qt::Key_F24: result = Input::KEY_F24; break;
    case Qt::Key_F25: result = Input::KEY_F25; break;
    //result = Input::KEY_KP_0; break;
    //result = Input::KEY_KP_1; break;
    //result = Input::KEY_KP_2; break;
    //result = Input::KEY_KP_3; break;
    //result = Input::KEY_KP_4; break;
    //result = Input::KEY_KP_5; break;
    //result = Input::KEY_KP_6; break;
    //result = Input::KEY_KP_7; break;
    //result = Input::KEY_KP_8; break;
    //result = Input::KEY_KP_9; break;
    //result = Input::KEY_KP_DECIMAL; break;
    //result = Input::KEY_KP_DIVIDE; break;
    //result = Input::KEY_KP_MULTIPLY; break;
    //result = Input::KEY_KP_SUBTRACT; break;
    //result = Input::KEY_KP_ADD; break;
    //result = Input::KEY_KP_ENTER; break;
    //result = Input::KEY_KP_EQUAL; break;
    case Qt::Key_Shift: result = Input::KEY_LEFT_SHIFT; break;
    case Qt::Key_Control: result = Input::KEY_LEFT_CONTROL; break;
    case Qt::Key_Alt: result = Input::KEY_LEFT_ALT; break;
    case Qt::Key_Super_L: result = Input::KEY_LEFT_SUPER; break;
    //result = Input::KEY_RIGHT_SHIFT; break;
    //result = Input::KEY_RIGHT_CONTROL; break;
    //result = Input::KEY_RIGHT_ALT; break;
    case Qt::Key_Super_R: result = Input::KEY_RIGHT_SUPER; break;
    case Qt::Key_Menu: result = Input::KEY_MENU; break;
    default: break;
    }

    return result;
}

bool SceneView::key(Input::KeyCode code) {
    return (m_Keys[code] > RELEASE);
}

bool SceneView::keyPressed(Input::KeyCode code) {
    return (m_Keys[code] == PRESS);
}

bool SceneView::keyReleased(Input::KeyCode code) {
    return (m_Keys[code] == RELEASE);
}

bool SceneView::mouseButton(Input::MouseButton button) {
    return (m_MouseButtons[button] > RELEASE);
}

bool SceneView::mousePressed(Input::MouseButton button) {
    return (m_MouseButtons[button] == PRESS);
}

bool SceneView::mouseReleased(Input::MouseButton button) {
    return (m_MouseButtons[button] == RELEASE);
}

void SceneView::mousePressEvent(QMouseEvent *ev) {
    Input::MouseButton btn = Input::LEFT;
    switch(ev->button()) {
        case Qt::LeftButton: btn = Input::LEFT; break;
        case Qt::RightButton: btn = Input::RIGHT; break;
        case Qt::MiddleButton: btn = Input::MIDDLE; break;
        case Qt::ExtraButton1: btn = Input::BUTTON0; break;
        case Qt::ExtraButton2: btn = Input::BUTTON1; break;
        case Qt::ExtraButton3: btn = Input::BUTTON2; break;
        case Qt::ExtraButton4: btn = Input::BUTTON3; break;
        case Qt::ExtraButton5: btn = Input::BUTTON4; break;
        default: break;
    }
    m_MouseButtons[btn] = PRESS;
}

void SceneView::mouseReleaseEvent(QMouseEvent *ev) {
    Input::MouseButton btn = Input::LEFT;
    switch(ev->button()) {
        case Qt::LeftButton: btn = Input::LEFT; break;
        case Qt::RightButton: btn = Input::RIGHT; break;
        case Qt::MiddleButton: btn = Input::MIDDLE; break;
        case Qt::ExtraButton1: btn = Input::BUTTON0; break;
        case Qt::ExtraButton2: btn = Input::BUTTON1; break;
        case Qt::ExtraButton3: btn = Input::BUTTON2; break;
        case Qt::ExtraButton4: btn = Input::BUTTON3; break;
        case Qt::ExtraButton5: btn = Input::BUTTON4; break;
        default: break;
    }
    m_MouseButtons[btn] = RELEASE;
}

void SceneView::keyPressEvent(QKeyEvent *ev) {
    QOpenGLWidget::keyPressEvent(ev);
    m_Keys[mapToInput(ev->key())] = ev->isAutoRepeat() ? REPEAT : PRESS;
}

void SceneView::keyReleaseEvent(QKeyEvent *ev) {
    QOpenGLWidget::keyReleaseEvent(ev);
    m_Keys[mapToInput(ev->key())] = RELEASE;
}

void SceneView::findCamera() {
    Actor *chunk    = m_pScene->findChild<Actor *>(false);
    if(chunk) {
        Camera *camera  = chunk->findChild<Camera *>();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
            pipe->setTarget(defaultFramebufferObject());
        }
        Camera::setCurrent(camera);
    }
}
