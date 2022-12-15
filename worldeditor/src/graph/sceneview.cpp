#include "sceneview.h"

#include <QWindow>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <engine.h>
#include <editor/editorplatform.h>
#include <systems/rendersystem.h>

SceneView::SceneView(QWidget *parent) :
        QWidget(parent),
        m_rhiWindow(nullptr),
        m_engine(nullptr),
        m_gamePause(false) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    m_rhiWindow = Engine::renderSystem()->createRhiWindow();
    m_rhiWindow->installEventFilter(this);

    connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

    layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));
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
        RenderSystem *system = Engine::renderSystem();
        if(system && system->pipelineContext() == nullptr) {
            system->init();
        }

        EditorPlatform &instance = EditorPlatform::instance();

        instance.setScreenSize(size());
        QPoint p = mapFromGlobal(QCursor::pos());
        QPoint pos = QPoint(p.x(), height() - p.y());
        instance.setMousePosition(pos);
        QPoint delta(p - m_savedMousePos);
        instance.setMouseDelta(QPoint(delta.x(), -delta.y()));

        m_engine->update();

        if(instance.isMouseLocked() && Engine::isGameMode()) {
            m_savedMousePos = QPoint(width() / 2, height() / 2);

            QCursor::setPos(mapToGlobal(m_savedMousePos));
            m_rhiWindow->setCursor(Qt::BlankCursor);
        } else {
            m_savedMousePos = p;
            m_rhiWindow->setCursor(Qt::ArrowCursor);
        }
    }
}

bool SceneView::eventFilter(QObject *object, QEvent *event) {
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
