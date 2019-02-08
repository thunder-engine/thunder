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

QString PathEdit::data() const {
    return ui->lineEdit->text();
}

void PathEdit::setData(const QString &v) {
    ui->lineEdit->setText(v);
}

void PathEdit::onFileDialog() {
    QString current = ProjectManager::instance()->contentPath();

    QString path = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(parent()),
                                                tr("Select File"),
                                                current,
                                                tr("All Files (*.*)"));

    if(path.length() > 0) {
        path = QDir(current).relativeFilePath(path);

        emit pathChanged(path);
        ui->lineEdit->setText(path);
    }
}
