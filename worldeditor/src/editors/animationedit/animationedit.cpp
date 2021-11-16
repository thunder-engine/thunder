#include "animationedit.h"
#include "ui_animationedit.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QMenu>
#include <QWidgetAction>

#include "animationbuilder.h"

#include <components/animator.h>
#include <resources/animationstatemachine.h>

#include "editors/componentbrowser/componentbrowser.h"

AnimationEdit::AnimationEdit() :
        ui(new Ui::AnimationEdit),
        m_schemeModel(new AnimationBuilder()),
        m_stateMachine(nullptr),
        m_createMenu(new QMenu(this)),
        m_browser(new ComponentBrowser(this)),
        m_selectedItem(nullptr),
        m_node(-1),
        m_port(-1),
        m_out(false),
        m_modified(false) {

    ui->setupUi(this);

    connect(m_schemeModel, SIGNAL(schemeUpdated()), this, SLOT(onUpdateAsset()));
    connect(m_schemeModel, SIGNAL(nodeMoved()), this, SLOT(onUpdateAsset()));

    ui->quickWidget->rootContext()->setContextProperty("schemeModel", m_schemeModel);
    ui->quickWidget->rootContext()->setContextProperty("stateMachine", true);
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    ui->quickWidget->setWindowTitle("Scheme");

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));
    connect(item, SIGNAL(showContextMenu(int,int,bool)), this, SLOT(onShowContextMenu(int,int,bool)));

    m_browser->setModel(m_schemeModel->components());
    connect(m_browser, SIGNAL(componentSelected(QString)), this, SLOT(onComponentSelected(QString)));

    QWidgetAction *action = new QWidgetAction(m_createMenu);
    action->setDefaultWidget(m_browser);
    m_createMenu->addAction(action);
}

AnimationEdit::~AnimationEdit() {
    delete ui;
}

bool AnimationEdit::isModified() const {
    return m_modified;
}

QStringList AnimationEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_schemeModel)->suffixes();
}

void AnimationEdit::onActivated() {
    emit itemSelected(m_selectedItem);
}

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        m_pSettings = settings;

        m_stateMachine = Engine::loadResource<AnimationStateMachine>(qPrintable(settings->destination()));

        m_schemeModel->load(m_pSettings->source());

        onUpdateAsset(false);
        onNodesSelected(QVariantList({0}));
    }
}

void AnimationEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_pSettings->source().isEmpty()) {
        m_schemeModel->save(path.isEmpty() ? m_pSettings->source() : path);
        onUpdateAsset(false);
    }
}

void AnimationEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_schemeModel->node(list.front().toInt());
        if(node) {
            m_selectedItem = static_cast<QObject *>(node->ptr);
            emit itemSelected(m_selectedItem);
        }
    }
}

void AnimationEdit::onUpdateAsset(bool update) {
    if(m_schemeModel) {
        m_modified = update;
        QString title(tr("Animation Editor"));
        if(m_modified) {
            title.append('*');
        }
        setWindowTitle(title);

        emit updateAsset();
    }
}

void AnimationEdit::onComponentSelected(const QString &path) {
    m_createMenu->hide();

    QQuickItem *scheme = ui->quickWidget->rootObject()->findChild<QQuickItem *>("Scheme");
    if(scheme) {
        int x = scheme->property("x").toInt();
        int y = scheme->property("y").toInt();
        float scale = scheme->property("scale").toFloat();

        QQuickItem *canvas = ui->quickWidget->rootObject()->findChild<QQuickItem *>("Canvas");
        if(canvas) {
            int mouseX = canvas->property("mouseX").toInt();
            int mouseY = canvas->property("mouseY").toInt();
            x = (float)(mouseX - x) * scale;
            y = (float)(mouseY - y) * scale;

            if(m_node > -1) {
                m_schemeModel->createAndLink(path, x, y, m_node, m_port, m_out);
            } else {
                m_schemeModel->createNode(path, x, y);
            }
        }
    }
}

void AnimationEdit::onShowContextMenu(int node, int port, bool out) {
    m_node = node;
    m_port = port;
    m_out = out;
    m_createMenu->exec(QCursor::pos());
}

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
