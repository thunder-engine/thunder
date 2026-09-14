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
#include "editorsettingsbrowser.h"
#include "ui_editorsettingsbrowser.h"

#include <QEvent>
#include <QSettings>
#include <QStringListModel>

#include <editor/editorsettings.h>

class StringListModel : public QStringListModel {
    QVariant data(const QModelIndex &index, int role) const {
        switch(role) {
            case Qt::BackgroundRole: {
                return QApplication::palette("QTreeView").brush(QPalette::Normal, QPalette::Button).color();
            } break;
            case Qt::FontRole: {
                    QFont font = QApplication::font("QTreeView");
                    font.setBold(true);
                    font.setPointSize(10);
                    return font;
            } break;
            case Qt::SizeHintRole: {
                return QSize(1, 26);
            }
            default: break;
        }

        return QStringListModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
};

EditorSettingsBrowser::EditorSettingsBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::EditorSettingsBrowser),
        m_groupModel(new StringListModel) {

    ui->setupUi(this);

    ui->groups->setModel(m_groupModel);

    connect(ui->propertiesWidget, &PropertyEditor::objectsChanged, this, &EditorSettingsBrowser::onSettingsUpdated);
}

EditorSettingsBrowser::~EditorSettingsBrowser() {
    delete ui;
}

void EditorSettingsBrowser::init() {
    ui->propertiesWidget->onObjectSelected(Editor::settings());

    QStringList groups;
    QAbstractItemModel *m = ui->propertiesWidget->model();
    for(uint32_t i = 0; i < m->rowCount(); i++) {
        QModelIndex index = m->index(i, 0, QModelIndex());
        groups << m->data(index).toString();
    }

    static_cast<StringListModel *>(m_groupModel)->setStringList(groups);

    ui->propertiesWidget->setGroup(groups.front());
    ui->groups->setCurrentIndex(m_groupModel->index(0, 0));
}

void EditorSettingsBrowser::onSettingsUpdated(const Object::ObjectList &objects, const TString &property, Variant value) {
    Editor::settings()->setValue(property, value);
}

void EditorSettingsBrowser::on_groups_clicked(const QModelIndex &index) {
    ui->propertiesWidget->setGroup(ui->groups->model()->data(index).toString());
}

void EditorSettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

