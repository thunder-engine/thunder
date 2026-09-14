/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
    dialog.setDirectory(url.dir().isEmpty() ? Editor::project()->contentPath() : url.absoluteDir());

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
