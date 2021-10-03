#include "animationedit.h"
#include "ui_animationedit.h"

#include <QQmlContext>
#include <QQuickItem>

#include "animationbuilder.h"

#include <components/animationcontroller.h>
#include <resources/animationstatemachine.h>

AnimationEdit::AnimationEdit() :
        m_Modified(false),
        ui(new Ui::AnimationEdit),
        m_pBuilder(new AnimationBuilder()),
        m_pMachine(nullptr) {

    ui->setupUi(this);

    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateAsset()));
    connect(m_pBuilder, SIGNAL(nodeMoved()), this, SLOT(onUpdateAsset()));

    ui->components->setModel(m_pBuilder->components());

    ui->quickWidget->rootContext()->setContextProperty("schemeModel", m_pBuilder);
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
    return m_Modified;
}

QStringList AnimationEdit::suffixes() const {
    return static_cast<AssetConverter *>(m_pBuilder)->suffixes();
}

void AnimationEdit::loadAsset(AssetConverterSettings *settings) {
    if(m_pSettings != settings) {
        m_pSettings = settings;

        m_pMachine = Engine::loadResource<AnimationStateMachine>(qPrintable(settings->destination()));

        m_pBuilder->load(m_pSettings->source());

        onUpdateAsset(false);
        onNodesSelected(QVariantList({0}));
    }
}

void AnimationEdit::saveAsset(const QString &path) {
    if(!path.isEmpty() || !m_pSettings->source().isEmpty()) {
        m_pBuilder->save(path.isEmpty() ? m_pSettings->source() : path);
        onUpdateAsset(false);
    }
}

void AnimationEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_pBuilder->node(list.front().toInt());
        if(node) {
            emit itemSelected(static_cast<QObject *>(node->ptr));
        }
    }
}

void AnimationEdit::onUpdateAsset(bool update) {
    if(m_pBuilder) {
        m_Modified = update;
        QString title(tr("Animation Editor"));
        if(m_Modified) {
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
