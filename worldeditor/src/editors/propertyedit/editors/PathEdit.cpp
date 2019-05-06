#include "PathEdit.h"
#include "ui_PathEdit.h"

#include <QFileDialog>

#include "projectmanager.h"

PathEdit::PathEdit(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PathEdit) {

    ui->setupUi(this);

    connect(ui->toolButton, SIGNAL(clicked()), this, SLOT(onFileDialog()));
}

QFileInfo PathEdit::data() const {
    return m_Info;
}

void PathEdit::setData(const QFileInfo &v) {
    m_Info = v;
    ui->lineEdit->setText(m_Info.filePath());
    emit pathChanged(v.filePath());
}

void PathEdit::onFileDialog() {
    QString current = ProjectManager::instance()->contentPath();

    QString path;
    if(m_Info.isDir()) {
        path = QFileDialog::getExistingDirectory(dynamic_cast<QWidget *>(parent()),
                                                 tr("Open Directory"),
                                                 "/",
                                                 QFileDialog::ShowDirsOnly);
    } else {
        path = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(parent()),
                                            tr("Select File"),
                                            current,
                                            tr("All Files (*.*)"));

        path = QDir(current).relativeFilePath(path);
    }

    if(path.length() > 0) {
        setData(path);
    }
}
