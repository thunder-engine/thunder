#include "animationedit.h"
#include "ui_animationedit.h"

#include <QQmlContext>
#include <QQuickItem>

#include "animationbuilder.h"

#include <components/animationcontroller.h>
#include <resources/animationstatemachine.h>

AnimationEdit::AnimationEdit() :
        m_modified(false),
        ui(new Ui::AnimationEdit),
        m_schemeModel(new AnimationBuilder()),
        m_stateMachine(nullptr),
        m_selectedItem(nullptr) {

    ui->setupUi(this);

    connect(m_schemeModel, SIGNAL(schemeUpdated()), this, SLOT(onUpdateAsset()));
    connect(m_schemeModel, SIGNAL(nodeMoved()), this, SLOT(onUpdateAsset()));

    ui->components->setModel(m_schemeModel->components());

    ui->quickWidget->rootContext()->setContextProperty("schemeModel", m_schemeModel);
    ui->quickWidget->rootContext()->setContextProperty("stateMachine", true);
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/SchemeEditor.qml"));

    ui->quickWidget->setWindowTitle("Scheme");

    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(nodesSelected(QVariant)), this, SLOT(onNodesSelected(QVariant)));

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
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

void AnimationEdit::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
