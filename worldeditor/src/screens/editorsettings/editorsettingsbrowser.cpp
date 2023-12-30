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
}

EditorSettingsBrowser::~EditorSettingsBrowser() {
    delete ui;
}

void EditorSettingsBrowser::onSettingsUpdated() {
    ui->propertiesWidget->onItemsSelected({EditorSettings::instance()});

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

void EditorSettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void EditorSettingsBrowser::on_groups_clicked(const QModelIndex &index) {
    ui->propertiesWidget->setGroup(ui->groups->model()->data(index).toString());
}
