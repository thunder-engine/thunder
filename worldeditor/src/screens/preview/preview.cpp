#include "preview.h"
#include "ui_preview.h"

#include <QWindow>

#include <QGuiApplication>
#include <QLayout>
#include <QMouseEvent>

#include <engine.h>
#include <editor/editorplatform.h>
#include <systems/rendersystem.h>

Preview::Preview(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::Preview),
        m_rhiWindow(nullptr),
        m_engine(nullptr),
        m_gamePause(false) {

    ui->setupUi(this);

    m_rhiWindow = Engine::renderSystem()->createRhiWindow();
    m_rhiWindow->installEventFilter(this);

    connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

    layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));
}

void Preview::setEngine(Engine *engine) {
    m_engine = engine;
}

bool Preview::isGamePause() const {
    return m_gamePause;
}

void Preview::setGamePause(bool pause) {
    m_gamePause = pause;
}

void Preview::onDraw() {
    if(m_engine && !m_gamePause && m_rhiWindow == QGuiApplication::focusWindow()) {
        RenderSystem *system = Engine::renderSystem();
        if(system && system->pipelineContext() == nullptr) {
            system->init();
        }

        EditorPlatform &instance = EditorPlatform::instance();

        instance.setScreenSize(size());
        QPoint p = mapFromGlobal(QCursor::pos());
        instance.setMousePosition(p);
        instance.setMouseDelta(p - m_savedMousePos);

        m_engine->update();

        if(instance.isMouseLocked() && Engine::isGameMode()) {
            m_savedMousePos = QPoint(width() / 2, height() / 2);

            QCursor::setPos(mapToGlobal(m_savedMousePos));
            m_rhiWindow->setCursor(Qt::BlankCursor);
        } else {
            m_savedMousePos = p;
            m_rhiWindow->setCursor(Qt::ArrowCursor);
        }
    } else {
        m_rhiWindow->setCursor(Qt::ArrowCursor);
    }
}

bool Preview::eventFilter(QObject *object, QEvent *event) {
    switch(event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), ev->text(), false, ev->isAutoRepeat());
            return true;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), "", true, ev->isAutoRepeat());
            return true;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), PRESS);
            return true;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMouseButtons(ev->button(), RELEASE);
            return true;
        }
        default: break;
    }
    return QObject::eventFilter(object, event);
}
