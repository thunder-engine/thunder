#include "pathedit.h"
#include "ui_pathedit.h"

#include <file.h>
#include <url.h>
#include <filedialog.h>

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

    FileDialog dialog;
    dialog.setDirectory(url.dir().isEmpty() ? ProjectSettings::instance()->contentPath() : url.absoluteDir());

    if(!m_file) {
        dialog.setWindowTitle("Open Directory");
        dialog.setMode(FileDialog::OpenDirectory);
    } else {
        dialog.setWindowTitle("Select File");
        dialog.setMode(FileDialog::OpenFile);
    }

    if(dialog.exec()) {
        TString path(dialog.getSelectedFile());
        if(!path.isEmpty()) {
            setData(path);
        }
    }
}

void PathEdit::onEditingFinished() {
    setData(TString(ui->lineEdit->text().toStdString()));
}
