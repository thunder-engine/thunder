#include "pathedit.h"
#include "ui_pathedit.h"

#include <QFileDialog>

#include <editor/projectsettings.h>

PathEdit::PathEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::PathEdit) {

    ui->setupUi(this);

    connect(ui->toolButton, &QToolButton::clicked, this, &PathEdit::onFileDialog);
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &PathEdit::onEditingFinished);
}

QVariant PathEdit::data() const {
    return QVariant::fromValue(m_info);
}

void PathEdit::setData(const QVariant &data) {
    m_info = data.value<QFileInfo>();
    ui->lineEdit->setText(m_info.filePath());
    emit editFinished();
}

void PathEdit::onFileDialog() {
    QString path;
    if(m_info.isDir()) {
        path = QFileDialog::getExistingDirectory(dynamic_cast<QWidget *>(parent()),
                                                 tr("Open Directory"),
                                                 "/",
                                                 QFileDialog::ShowDirsOnly);
    } else {
        path = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(parent()),
                                            tr("Select File"),
                                            ProjectSettings::instance()->contentPath(),
                                            tr("All Files (*)"));
    }

    if(path.length() > 0) {
        setData(QVariant::fromValue<QFileInfo>(path));
    }
}

void PathEdit::onEditingFinished() {
    setData(QVariant::fromValue<QFileInfo>(ui->lineEdit->text()));
}
