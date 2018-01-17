#include "projectdialog.h"
#include "ui_projectdialog.h"

#include "common.h"
#include "projectmodel.h"

#include <QFileDialog>

ProjectDialog::ProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    ui->listView->setModel(new ProjectModel());
    ui->loadBtn->setEnabled(false);
}

ProjectDialog::~ProjectDialog()
{
    delete ui;
}

QString ProjectDialog::projectPath() {
    ProjectDialog dlg;
    QString result;
    if(dlg.exec() == Accepted) {
        result  = dlg.path();
    }
    return result;
}

QString ProjectDialog::path() const {
    return m_Path;
}

void ProjectDialog::on_listView_doubleClicked(const QModelIndex &index) {
    if(index.isValid()) {
        m_Path  = index.data(ProjectModel::PathRole).toString();
        accept();
    }
}

void ProjectDialog::on_listView_clicked(const QModelIndex &index) {
    ui->loadBtn->setEnabled(index.isValid());
}

void ProjectDialog::on_importBtn_clicked() {
    m_Path  = QFileDialog::getOpenFileName(this, tr("Import Existing Project"), "", "*" + gProjectExt);
    accept();
}

void ProjectDialog::on_loadBtn_clicked() {
    QItemSelectionModel *m  = ui->listView->selectionModel();
    if(m) {
        QModelIndex index   = m->currentIndex();
        if(index.isValid()) {
            m_Path  = index.data(ProjectModel::PathRole).toString();
            accept();
        }
    }
}

void ProjectDialog::on_newBtn_clicked() {
    QString path    = QFileDialog::getSaveFileName(this, tr("Create New Project"), "", "*" + gProjectExt);
    if(!path.isEmpty()) {
        path   += gProjectExt;
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            file.close();
            m_Path  = path;
            accept();
        }
    }
}
