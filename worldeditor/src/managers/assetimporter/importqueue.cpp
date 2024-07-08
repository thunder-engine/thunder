#include "importqueue.h"
#include "ui_importqueue.h"

#include <QKeyEvent>
#include <QDir>
#include <QScreen>

#include <editor/projectsettings.h>

#include <editor/assetmanager.h>

#include "iconrender.h"

ImportQueue::ImportQueue(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ImportQueue),
        m_render(new IconRender) {
    ui->setupUi(this);

    AssetManager *manager = AssetManager::instance();
    connect(manager, &AssetManager::importStarted, this, &ImportQueue::onStarted);
    connect(manager, &AssetManager::imported, this, &ImportQueue::onProcessed);

    connect(manager, &AssetManager::importFinished, this, &ImportQueue::onImportFinished);

    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    QRect r = QApplication::screens().at(0)->geometry();
    move(r.center() - rect().center());
}

ImportQueue::~ImportQueue() {
    delete ui;
}

void ImportQueue::onProcessed(const QString &path, const QString &type) {
    ui->progressBar->setValue(ui->progressBar->value() + 1);

    QString guid = QString::fromStdString(AssetManager::instance()->pathToGuid(path.toStdString()));
    m_updateQueue[guid] = type;
}

void ImportQueue::onStarted(int count, const QString &action) {
    show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(count);
    ui->label->setText(action);
}

void ImportQueue::onImportFinished() {
    auto i = m_updateQueue.constBegin();
    while(i != m_updateQueue.constEnd()) {
        QImage image = m_render->render(i.key(), i.value());
        if(!image.isNull()) {
            image.save(ProjectSettings::instance()->iconPath() + QDir::separator() + i.key() + ".png", "PNG");
        }
        emit AssetManager::instance()->iconUpdated(i.key());
        ++i;
    }
    m_updateQueue.clear();

    hide();
    emit importFinished();
}

void ImportQueue::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) {
        e->ignore();
    }
}

void ImportQueue::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
