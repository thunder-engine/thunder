#include "projectbrowser.h"
#include "ui_projectbrowser.h"

#include "managers/projectmanager/projectmodel.h"
#include "editor/projectsettings.h"

#include <QKeyEvent>
#include <QFileDialog>

#include "config.h"

ProjectBrowser::ProjectBrowser(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ProjectBrowser) {

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    ui->listView->setModel(new ProjectModel);

    connect(ui->newButton, &QPushButton::clicked, this, &ProjectBrowser::onNewProject);
    connect(ui->importButton, &QPushButton::clicked, this, &ProjectBrowser::onImportProject);
    connect(ui->openButton, &QPushButton::clicked, this, &ProjectBrowser::onOpenProject);

    connect(ui->listView, &QListView::doubleClicked, this, &ProjectBrowser::onProjectSelected);
}

ProjectBrowser::~ProjectBrowser() {
    delete ui;
}

void ProjectBrowser::onNewProject() {
    m_projectPath = QFileDialog::getSaveFileName(this, tr("Create New Project"),
                                                 ProjectSettings::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!m_projectPath.isEmpty()) {
        QFileInfo info(m_projectPath);
        if(info.suffix().isEmpty()) {
            m_projectPath += gProjectExt;
        }
        QFile file(m_projectPath);
        if(file.open(QIODevice::WriteOnly)) {
            file.close();

            done(QDialog::Accepted);
        }
    }
}

void ProjectBrowser::onImportProject() {
    m_projectPath = QFileDialog::getOpenFileName(this, tr("Import Existing Project"),
                                                 ProjectSettings::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!m_projectPath.isEmpty()) {
        done(QDialog::Accepted);
    }
}

void ProjectBrowser::onOpenProject() {
    QItemSelectionModel *selection = ui->listView->selectionModel();
    if(selection->hasSelection()) {
        onProjectSelected(selection->selectedIndexes().front());
    }
}

void ProjectBrowser::onProjectSelected(const QModelIndex &index) {
    m_projectPath = ui->listView->model()->data(index, Qt::EditRole).toString();
    if(!m_projectPath.isEmpty()) {
        done(QDialog::Accepted);
    }
}

void ProjectBrowser::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    }
}
