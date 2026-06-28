#include "projectbrowser.h"
#include "ui_projectbrowser.h"

#include "managers/projectmanager/projectmodel.h"
#include "editor/projectsettings.h"

#include <QKeyEvent>

#include <file.h>
#include <url.h>
#include <filedialog.h>

#include "config.h"

ProjectBrowser::ProjectBrowser(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ProjectBrowser) {

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
    FileDialog dialog;
    dialog.setDirectory(ProjectSettings::instance()->myProjectsPath());
    dialog.setWindowTitle("Create New Project");
    dialog.setMode(FileDialog::SaveFile);
    dialog.addFilter("Project Files", { TString("*") + gProjectExt });

    m_projectPath.clear();
    if(dialog.exec()) {
        m_projectPath = dialog.getSelectedFile();
    }

    if(!m_projectPath.isEmpty()) {
        Url info(m_projectPath);
        if(info.suffix().isEmpty()) {
            m_projectPath += gProjectExt;
        }
        File file(m_projectPath);
        if(file.open(File::Write)) {
            file.close();

            done(QDialog::Accepted);
        }
    }
}

void ProjectBrowser::onImportProject() {
    FileDialog dialog;
    dialog.setDirectory(ProjectSettings::instance()->myProjectsPath());
    dialog.setWindowTitle("Import Existing Project");
    dialog.setMode(FileDialog::OpenFile);
    dialog.addFilter("Project Files", { TString("*") + gProjectExt });

    m_projectPath.clear();
    if(dialog.exec()) {
        m_projectPath = dialog.getSelectedFile();
    }

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
    m_projectPath = ui->listView->model()->data(index, Qt::EditRole).toString().toStdString();
    if(!m_projectPath.isEmpty()) {
        done(QDialog::Accepted);
    }
}

void ProjectBrowser::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    }
}
