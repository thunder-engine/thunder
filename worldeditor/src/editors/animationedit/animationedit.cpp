#include "animationedit.h"
#include "ui_animationedit.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QDir>

#include "animationbuilder.h"

#include "projectmanager.h"

#include <components/animationcontroller.h>
#include <resources/animationstatemachine.h>

AnimationEdit::AnimationEdit(DocumentModel *document) :
        QWidget(nullptr),
        m_Modified(false),
        ui(new Ui::AnimationEdit),
        m_pBuilder(new AnimationBuilder()),
        m_pMachine(nullptr),
        m_pDocument(document) {

    ui->setupUi(this);

    connect(m_pBuilder, SIGNAL(schemeUpdated()), this, SLOT(onUpdateTemplate()));
    connect(m_pBuilder, SIGNAL(nodeMoved()), this, SLOT(onUpdateTemplate()));

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

void AnimationEdit::closeEvent(QCloseEvent *event) {
    if(!m_pDocument->checkSave(this)) {
        event->ignore();
        return;
    }
    QDir dir(ProjectManager::instance()->contentPath());
    m_pDocument->closeFile(dir.relativeFilePath(m_Path));
}

bool AnimationEdit::isModified() const {
    return m_Modified;
}

QStringList AnimationEdit::assetTypes() const {
    return {"AnimationStateMachine"};
}

void AnimationEdit::onNodesSelected(const QVariant &indices) {
    QVariantList list = indices.toList();
    if(!list.isEmpty()) {
        const AbstractSchemeModel::Node *node = m_pBuilder->node(list.front().toInt());
        if(node) {
            m_pDocument->itemSelected(static_cast<QObject *>(node->ptr));
        }
    }
}

void AnimationEdit::loadAsset(IConverterSettings *settings) {
    show();
    if(m_Path != settings->source()) {
        m_Path = settings->source();
        m_pMachine = Engine::loadResource<AnimationStateMachine>(qPrintable(settings->destination()));

        m_pBuilder->load(m_Path);

        onUpdateTemplate(false);
        onNodesSelected(QVariantList({0}));
    }
}

void AnimationEdit::saveAsset(const QString &path) {
    m_Path = path;
    if(!m_Path.isEmpty()) {
        m_pBuilder->save(m_Path);
        onUpdateTemplate(false);
    }
}

void AnimationEdit::onUpdateTemplate(bool update) {
    if(m_pBuilder) {
        m_Modified = update;
        QString title(tr("Animation Editor"));
        if(m_Modified) {
            title.append('*');
        }
        setWindowTitle(title);
    }
}

void AnimationEdit::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
