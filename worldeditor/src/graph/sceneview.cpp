#include "sceneview.h"

#include <QWindow>
#include <QStandardPaths>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QGamepad>
#include <QGamepadManager>

#include <engine.h>
#include <timer.h>

#include <components/actor.h>
#include <components/scenegraph.h>
#include <components/camera.h>

#include <systems/rendersystem.h>

#include <editor/pluginmanager.h>

#define NONE 0
#define RELEASE 1
#define PRESS 2
#define REPEAT 3

Input::KeyCode mapToInput(int32_t key) {
    static const QMap<int32_t, uint32_t> map = {
        { Qt::Key_Space, Input::KEY_SPACE },
        { Qt::Key_Apostrophe, Input::KEY_APOSTROPHE },
        { Qt::Key_Comma, Input::KEY_COMMA },
        { Qt::Key_Minus, Input::KEY_MINUS },
        { Qt::Key_Period, Input::KEY_PERIOD },
        { Qt::Key_Slash, Input::KEY_SLASH },
        { Qt::Key_0, Input::KEY_0 },
        { Qt::Key_1, Input::KEY_1 },
        { Qt::Key_2, Input::KEY_2 },
        { Qt::Key_3, Input::KEY_3 },
        { Qt::Key_4, Input::KEY_4 },
        { Qt::Key_5, Input::KEY_5 },
        { Qt::Key_6, Input::KEY_6 },
        { Qt::Key_7, Input::KEY_7 },
        { Qt::Key_8, Input::KEY_8 },
        { Qt::Key_9, Input::KEY_9 },
        { Qt::Key_Semicolon, Input::KEY_SEMICOLON },
        { Qt::Key_Equal, Input::KEY_EQUAL },
        { Qt::Key_A, Input::KEY_A },
        { Qt::Key_B, Input::KEY_B },
        { Qt::Key_C, Input::KEY_C },
        { Qt::Key_D, Input::KEY_D },
        { Qt::Key_E, Input::KEY_E },
        { Qt::Key_F, Input::KEY_F },
        { Qt::Key_G, Input::KEY_G },
        { Qt::Key_H, Input::KEY_H },
        { Qt::Key_I, Input::KEY_I },
        { Qt::Key_J, Input::KEY_J },
        { Qt::Key_K, Input::KEY_K },
        { Qt::Key_L, Input::KEY_L },
        { Qt::Key_M, Input::KEY_M },
        { Qt::Key_N, Input::KEY_N },
        { Qt::Key_O, Input::KEY_O },
        { Qt::Key_P, Input::KEY_P },
        { Qt::Key_Q, Input::KEY_Q },
        { Qt::Key_R, Input::KEY_R },
        { Qt::Key_S, Input::KEY_S },
        { Qt::Key_T, Input::KEY_T },
        { Qt::Key_U, Input::KEY_U },
        { Qt::Key_V, Input::KEY_V },
        { Qt::Key_W, Input::KEY_W },
        { Qt::Key_X, Input::KEY_X },
        { Qt::Key_Y, Input::KEY_Y },
        { Qt::Key_Z, Input::KEY_Z },
        { Qt::Key_BracketLeft, Input::KEY_LEFT_BRACKET },
        { Qt::Key_Backslash, Input::KEY_BACKSLASH },
        { Qt::Key_BracketRight, Input::KEY_RIGHT_BRACKET },
        { Qt::Key_Agrave, Input::KEY_GRAVE_ACCENT },
        //result = Input::KEY_WORLD_1
        //result = Input::KEY_WORLD_2
        { Qt::Key_Escape, Input::KEY_ESCAPE },
        { Qt::Key_Enter, Input::KEY_ENTER },
        { Qt::Key_Tab, Input::KEY_TAB },
        { Qt::Key_Backspace, Input::KEY_BACKSPACE },
        { Qt::Key_Insert, Input::KEY_INSERT },
        { Qt::Key_Delete, Input::KEY_DELETE },
        { Qt::Key_Right, Input::KEY_RIGHT },
        { Qt::Key_Left, Input::KEY_LEFT },
        { Qt::Key_Down, Input::KEY_DOWN },
        { Qt::Key_Up, Input::KEY_UP },
        { Qt::Key_PageUp, Input::KEY_PAGE_UP },
        { Qt::Key_PageDown, Input::KEY_PAGE_DOWN },
        { Qt::Key_Home, Input::KEY_HOME },
        { Qt::Key_End, Input::KEY_END },
        { Qt::Key_CapsLock, Input::KEY_CAPS_LOCK },
        { Qt::Key_ScrollLock, Input::KEY_SCROLL_LOCK },
        { Qt::Key_NumLock, Input::KEY_NUM_LOCK },
        { Qt::Key_Print, Input::KEY_PRINT_SCREEN },
        { Qt::Key_Pause, Input::KEY_PAUSE },
        { Qt::Key_F1 , Input::KEY_F1 },
        { Qt::Key_F2 , Input::KEY_F2 },
        { Qt::Key_F3 , Input::KEY_F3 },
        { Qt::Key_F4 , Input::KEY_F4 },
        { Qt::Key_F5 , Input::KEY_F5 },
        { Qt::Key_F6 , Input::KEY_F6 },
        { Qt::Key_F7 , Input::KEY_F7 },
        { Qt::Key_F8 , Input::KEY_F8 },
        { Qt::Key_F9 , Input::KEY_F9 },
        { Qt::Key_F10, Input::KEY_F10 },
        { Qt::Key_F11, Input::KEY_F11 },
        { Qt::Key_F12, Input::KEY_F12 },
        { Qt::Key_F13, Input::KEY_F13 },
        { Qt::Key_F14, Input::KEY_F14 },
        { Qt::Key_F15, Input::KEY_F15 },
        { Qt::Key_F16, Input::KEY_F16 },
        { Qt::Key_F17, Input::KEY_F17 },
        { Qt::Key_F18, Input::KEY_F18 },
        { Qt::Key_F19, Input::KEY_F19 },
        { Qt::Key_F20, Input::KEY_F20 },
        { Qt::Key_F21, Input::KEY_F21 },
        { Qt::Key_F22, Input::KEY_F22 },
        { Qt::Key_F23, Input::KEY_F23 },
        { Qt::Key_F24, Input::KEY_F24 },
        { Qt::Key_F25, Input::KEY_F25 },
        //result = Input::KEY_KP_0
        //result = Input::KEY_KP_1
        //result = Input::KEY_KP_2
        //result = Input::KEY_KP_3
        //result = Input::KEY_KP_4
        //result = Input::KEY_KP_5
        //result = Input::KEY_KP_6
        //result = Input::KEY_KP_7
        //result = Input::KEY_KP_8
        //result = Input::KEY_KP_9
        //result = Input::KEY_KP_DECIMAL
        //result = Input::KEY_KP_DIVIDE
        //result = Input::KEY_KP_MULTIPLY
        //result = Input::KEY_KP_SUBTRACT
        //result = Input::KEY_KP_ADD
        //result = Input::KEY_KP_ENTER
        //result = Input::KEY_KP_EQUAL
        { Qt::Key_Shift, Input::KEY_LEFT_SHIFT },
        { Qt::Key_Control, Input::KEY_LEFT_CONTROL },
        { Qt::Key_Alt, Input::KEY_LEFT_ALT },
        { Qt::Key_Super_L, Input::KEY_LEFT_SUPER },
        //result = Input::KEY_RIGHT_SHIFT
        //result = Input::KEY_RIGHT_CONTROL
        //result = Input::KEY_RIGHT_ALT
        { Qt::Key_Super_R, Input::KEY_RIGHT_SUPER },
        { Qt::Key_Menu, Input::KEY_MENU },
    };

    return (Input::KeyCode)map.value(key, Input::KEY_UNKNOWN);
}

SceneView::SceneView(QWidget *parent) :
        QWidget(parent),
        m_engine(nullptr),
        m_gamepad(new QGamepad),
        m_mouseLock(false),
        m_gamePause(false) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    connect(m_gamepad, &QGamepad::connectedChanged, this, &SceneView::onGamepadConnected);

    m_rhiWindow = Engine::renderSystem()->createRhiWindow();
    m_rhiWindow->installEventFilter(this);

    connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

    layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

    Engine::setPlatformAdaptor(this);
    Input::init(this);
}

SceneView::~SceneView() {
    Engine::setPlatformAdaptor(nullptr);
}

void SceneView::setEngine(Engine *engine) {
    m_engine = engine;
}

bool SceneView::isGamePause() const {
    return m_gamePause;
}

void SceneView::setGamePause(bool pause) {
    m_gamePause = pause;
}

void SceneView::onDraw() {
    if(m_engine && !m_gamePause) {
        m_engine->update();
    }
}

void SceneView::onGamepadConnected(bool value) {

}

void SceneView::update() {
    m_inputString.clear();

    for(auto &it : m_keys) {
        switch(it) {
            case RELEASE: it = NONE; break;
            case PRESS: it = REPEAT; break;
            default: break;
        }
    }

    for(auto &it : m_mouseButtons) {
        switch(it) {
            case RELEASE: it = NONE; break;
            case PRESS: it = REPEAT; break;
            default: break;
        }
    }

    if(m_mouseLock && Engine::isGameMode()) {
        m_saved = QPoint(width() / 2, height() / 2);

        QCursor::setPos(mapToGlobal(m_saved));
        m_rhiWindow->setCursor(Qt::BlankCursor);
    } else {
        m_rhiWindow->setCursor(Qt::ArrowCursor);
    }

    PlatformAdaptor::update();

    m_mouseDelta = Vector4();
}

bool SceneView::key(Input::KeyCode code) const {
    return (m_keys.value(code) > RELEASE);
}

bool SceneView::keyPressed(Input::KeyCode code) const {
    return (m_keys.value(code) == PRESS);
}

bool SceneView::keyReleased(Input::KeyCode code) const {
    return (m_keys.value(code) == RELEASE);
}

string SceneView::inputString() const {
    return m_inputString;
}

bool SceneView::mouseButton(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) > RELEASE);
}

bool SceneView::mousePressed(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) == PRESS);
}

bool SceneView::mouseReleased(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) == RELEASE);
}

bool SceneView::eventFilter(QObject *object, QEvent *event) {
    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            Input::KeyCode code = mapToInput(ev->key());
            m_keys[code] = ev->isAutoRepeat() ? REPEAT : PRESS;
            m_inputString += ev->text().toStdString();
            return true;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            Input::KeyCode code = mapToInput(ev->key());
            m_keys[code] = RELEASE;
            return true;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            Input::KeyCode btn = Input::MOUSE_LEFT;
            switch(ev->button()) {
                case Qt::LeftButton: btn = Input::MOUSE_LEFT; break;
                case Qt::RightButton: btn = Input::MOUSE_RIGHT; break;
                case Qt::MiddleButton: btn = Input::MOUSE_MIDDLE; break;
                case Qt::ExtraButton1: btn = Input::MOUSE_BUTTON0; break;
                case Qt::ExtraButton2: btn = Input::MOUSE_BUTTON1; break;
                case Qt::ExtraButton3: btn = Input::MOUSE_BUTTON2; break;
                case Qt::ExtraButton4: btn = Input::MOUSE_BUTTON3; break;
                case Qt::ExtraButton5: btn = Input::MOUSE_BUTTON4; break;
                default: break;
            }
            m_mouseButtons[btn] = PRESS;
            return true;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            Input::KeyCode btn = Input::MOUSE_LEFT;
            switch(ev->button()) {
                case Qt::LeftButton: btn = Input::MOUSE_LEFT; break;
                case Qt::RightButton: btn = Input::MOUSE_RIGHT; break;
                case Qt::MiddleButton: btn = Input::MOUSE_MIDDLE; break;
                case Qt::ExtraButton1: btn = Input::MOUSE_BUTTON0; break;
                case Qt::ExtraButton2: btn = Input::MOUSE_BUTTON1; break;
                case Qt::ExtraButton3: btn = Input::MOUSE_BUTTON2; break;
                case Qt::ExtraButton4: btn = Input::MOUSE_BUTTON3; break;
                case Qt::ExtraButton5: btn = Input::MOUSE_BUTTON4; break;
                default: break;
            }
            m_mouseButtons[btn] = RELEASE;
            return true;
        }
        case QEvent::MouseMove: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            QPoint pos = ev->pos();
            QPoint delta = pos - m_saved;

            m_mouseDelta = Vector4(delta.x(), delta.y(),
                                  (float)delta.x() / (float)width(),
                                  (float)delta.y() / (float)height());
            if(!m_mouseLock) {
                m_saved = pos;
            }
            return true;
        }
        default: break;
    }
    return QObject::eventFilter(object, event);
}

Vector4 SceneView::mousePosition() const {
    QPoint p = mapFromGlobal(QCursor::pos());
    return Vector4(p.x(), height() - p.y(),
                   static_cast<float>(p.x()) / width(),
                   static_cast<float>(height() - p.y()) / height());
}

Vector4 SceneView::mouseDelta() const {
    return m_mouseDelta;
}

void SceneView::mouseLockCursor(bool lock) {
    m_mouseLock = lock;
}

uint32_t SceneView::screenWidth() const { return width(); }
uint32_t SceneView::screenHeight() const { return height(); }
uint32_t SceneView::joystickCount() const { return QGamepadManager::instance()->connectedGamepads().size(); }
uint32_t SceneView::joystickButtons(int) const { return 0; }
Vector4 SceneView::joystickThumbs(int) const { return Vector4(); }
Vector2 SceneView::joystickTriggers(int) const { return Vector2(); }
string SceneView::locationLocalDir() const { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(); }
