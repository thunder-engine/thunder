#include "pathedit.h"
#include "ui_pathedit.h"

#include <QFileDialog>

#include <file.h>
#include <url.h>

#include <editor/projectsettings.h>

PathEdit::PathEdit(bool file, QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::PathEdit),
        m_file(file) {

    ui->setupUi(this);

    connect(ui->toolButton, &QToolButton::clicked, this, &PathEdit::onFileDialog);
    connect(ui->lineEdit, &QLineEdit::editingFinished, this, &PathEdit::onEditingFinished);
}

Variant PathEdit::data() const {
    return m_path;
}

void PathEdit::setData(const Variant &data) {
    m_path = data.toString();
    ui->lineEdit->setText(m_path.data());
    emit editFinished();
}

void PathEdit::onFileDialog() {
    Url url(m_path);

    QString path;
    QString dir(url.dir().isEmpty() ? ProjectSettings::instance()->contentPath().data() : url.absoluteDir().data());

    if(!m_file) {
        path = QFileDialog::getExistingDirectory(dynamic_cast<QWidget *>(parent()),
                                                 tr("Open Directory"),
                                                 dir,
                                                 QFileDialog::ShowDirsOnly);
    } else {
        path = QFileDialog::getOpenFileName(dynamic_cast<QWidget *>(parent()),
                                            tr("Select File"),
                                            dir,
                                            tr("All Files (*)"));
    }

    if(path.length() > 0) {
        setData(TString(path.toStdString()));
    }
}

void PathEdit::onEditingFinished() {
    setData(TString(ui->lineEdit->text().toStdString()));
}
