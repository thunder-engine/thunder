#include <editor/editorplatform.h>

#include <QWindow>

#include <QGuiApplication>
#include <QStandardPaths>
#include <QMouseEvent>
#include <QGamepad>
#include <QGamepadManager>

#include <engine.h>
#include <components/camera.h>

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
        { Qt::Key_Return, Input::KEY_ENTER },
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
        {Qt::Key_Enter, Input::KEY_KP_ENTER },
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

EditorPlatform::EditorPlatform() :
        m_gamepad(new QGamepad),
        m_mouseLock(false) {

    //connect(m_gamepad, &QGamepad::connectedChanged, this, &EditorPlatform::onGamepadConnected);
}

EditorPlatform::~EditorPlatform() {

}

EditorPlatform &EditorPlatform::instance() {
    static EditorPlatform *instance = nullptr;
    if(instance == nullptr) {
        instance = new EditorPlatform;

        Engine::setPlatformAdaptor(instance);
        Input::init(instance);
    }

    return *instance;
}

bool EditorPlatform::init() {
    return true;
}

void EditorPlatform::update() {
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

    PlatformAdaptor::update();

    m_mouseDelta = Vector4();
}

bool EditorPlatform::key(Input::KeyCode code) const {
    return (m_keys.value(code) > RELEASE);
}

bool EditorPlatform::keyPressed(Input::KeyCode code) const {
    return (m_keys.value(code) == PRESS);
}

bool EditorPlatform::keyReleased(Input::KeyCode code) const {
    return (m_keys.value(code) == RELEASE);
}

string EditorPlatform::inputString() const {
    return m_inputString;
}

bool EditorPlatform::mouseButton(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) > RELEASE);
}

bool EditorPlatform::mousePressed(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) == PRESS);
}

bool EditorPlatform::mouseReleased(int button) const {
    return (m_mouseButtons.value(button | 0x10000000) == RELEASE);
}

void EditorPlatform::onGamepadConnected(bool value) {

}

void EditorPlatform::setScreenSize(const QSize &size) {
    m_screenSize = size;
}

void EditorPlatform::setMousePosition(const QPoint &position) {
    m_mousePosition = Vector4(position.x(),
                              position.y(),
                              position.x() / m_screenSize.width(),
                              1.0f - position.y() / m_screenSize.height());

    QPoint delta = position - m_saved;
    m_mouseDelta = Vector4(delta.x(), delta.y(),
                           delta.x() / m_screenSize.width(), delta.y() / m_screenSize.height());

    if(!m_mouseLock) {
        m_saved = position;
    } else {
        m_saved = QPoint(m_screenSize.width() / 2, m_screenSize.height() / 2);
    }
}

void EditorPlatform::setMouseButtons(int button, int state) {
    Input::KeyCode btn = Input::MOUSE_LEFT;

    switch(button) {
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

    m_mouseButtons[btn] = state;
}

void EditorPlatform::setKeys(int key, const QString &text, bool release, bool repeat) {
    Input::KeyCode code = mapToInput(key);
    m_keys[code] = release ? RELEASE : (repeat ? REPEAT : PRESS);
    m_inputString += text.toStdString();
}

bool EditorPlatform::isMouseLocked() const {
    return m_mouseLock;
}

Vector4 EditorPlatform::mousePosition() const {
    return m_mousePosition;
}

Vector4 EditorPlatform::mouseDelta() const {
    return m_mouseDelta;
}

void EditorPlatform::mouseLockCursor(bool lock) {
    m_mouseLock = lock;
}

uint32_t EditorPlatform::screenWidth() const { return m_screenSize.width(); }
uint32_t EditorPlatform::screenHeight() const { return m_screenSize.height(); }
uint32_t EditorPlatform::joystickCount() const { return QGamepadManager::instance()->connectedGamepads().size(); }
uint32_t EditorPlatform::joystickButtons(int) const { return 0; }
Vector4 EditorPlatform::joystickThumbs(int) const { return Vector4(); }
Vector2 EditorPlatform::joystickTriggers(int) const { return Vector2(); }
string EditorPlatform::locationLocalDir() const { return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(); }
