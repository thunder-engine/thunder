#include "importqueue.h"
#include "ui_importqueue.h"

#include <QKeyEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QOpenGLContext>

#include "assetmanager.h"
#include "projectmanager.h"
#include "codemanager.h"

#include "iconrender.h"

ImportQueue::ImportQueue(Engine *engine, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ImportQueue),
        m_pEngine(engine) {
    ui->setupUi(this);

    AssetManager *manager   = AssetManager::instance();
    connect(manager, SIGNAL(importStarted(int,QString)), this, SLOT(onStarted(int,QString)));
    connect(manager, SIGNAL(imported(QString,uint32_t)), this, SLOT(onProcessed(QString,uint32_t)));

    connect(manager, &AssetManager::importFinished, this, &ImportQueue::onFinished);
    connect(manager, &AssetManager::importFinished, CodeManager::instance(), &CodeManager::buildProject);

    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    QRect r = QApplication::desktop()->screenGeometry();
    move(r.center() - rect().center());
}

ImportQueue::~ImportQueue() {
    delete ui;
}

void ImportQueue::onProcessed(const QString &path, uint32_t type) {
    ui->progressBar->setValue(ui->progressBar->value() + 1);

    QString guid    = QString::fromStdString(AssetManager::instance()->pathToGuid(path.toStdString()));
    m_UpdateQueue[guid] = type;
}

void ImportQueue::onStarted(int count, const QString &action) {
    show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(count);
    ui->label->setText(action);
}

void ImportQueue::onFinished() {
    hide();

    IconRender render(m_pEngine, QOpenGLContext::globalShareContext());

    QMap<QString, uint8_t>::const_iterator i = m_UpdateQueue.constBegin();
    while(i != m_UpdateQueue.constEnd()) {
        QImage image = render.render(i.key(), i.value());
        if(!image.isNull()) {
            image.save(ProjectManager::instance()->iconPath() + QDir::separator() + i.key() + ".png", "PNG");
        }
        emit rendered(i.key());
        ++i;
    }
    m_UpdateQueue.clear();

    emit finished();
}

void ImportQueue::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) {
        e->ignore();
    }
}
