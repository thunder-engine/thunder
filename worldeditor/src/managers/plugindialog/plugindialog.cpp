#include <QFileDialog>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QDebug>

#include "ui_plugindialog.h"

#include "plugindialog.h"

#include <editor/pluginmanager.h>

#define ROW_HEIGHT 18
#define ICON_SIZE 14

class PluginDelegate : public QStyledItemDelegate {
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        const QPalette &palette(opt.palette);
        QRect rect(opt.rect);

        static QImage right(":/Style/styles/dark/icons/arrow-right.png");
        static QImage down(":/Style/styles/dark/icons/arrow-down.png");

        painter->save();
        painter->setClipping(true);
        painter->setClipRect(rect);
        painter->setFont(opt.font);

        painter->setPen(palette.color(QPalette::Shadow));

        QRect border(rect);
        border.adjust(1, 1,-1,-1);
        painter->drawRoundedRect(border, 4, 4);

        rect.setHeight(ROW_HEIGHT);
        rect.setLeft(20);

        painter->setPen(palette.windowText().color());
        painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, opt.text);

        rect.setLeft(5);

        QRect icon(0, rect.y() + (ROW_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);

        bool expanded = m_Expandes.value(index, false);
        if(expanded) {
            const QAbstractItemModel *m = index.model();

            rect.moveTop(rect.y() + ROW_HEIGHT);
            QString text = m->data(m->index(index.row(), PluginManager::PLUGIN_DESCRIPTION)).toString();
            painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, tr("Description: ") + text);
            rect.moveTop(rect.y() + ROW_HEIGHT);
            text = m->data(m->index(index.row(), PluginManager::PLUGIN_PATH)).toString();
            painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, tr("File: ") + text);
            rect.moveTop(rect.y() + ROW_HEIGHT);
            text = m->data(m->index(index.row(), PluginManager::PLUGIN_AUTHOR)).toString();
            painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, tr("Author: ") + text);
            rect.moveTop(rect.y() + ROW_HEIGHT);
            text = m->data(m->index(index.row(), PluginManager::PLUGIN_VERSION)).toString();
            painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, tr("Version: ") + text);
        }

        // Draw arrow
        painter->drawImage(icon, expanded ? down : right);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);

        size.setHeight(m_Expandes.value(index, false) ? (PluginManager::PLUGIN_LAST * ROW_HEIGHT) : ROW_HEIGHT);
        return size;
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        QRect rect(option.rect);

        QMouseEvent *ev = dynamic_cast<QMouseEvent *>(event);
        if(ev && ev->type() == QEvent::MouseButtonPress && ev->button() == Qt::LeftButton) {
            QRect icon(0, rect.y() + 5, 14, 14);
            if(icon.contains(ev->pos())) {
                m_Expandes[index] = !m_Expandes[index];
                emit model->layoutAboutToBeChanged();
                emit model->layoutChanged();
            }
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    QMap<QModelIndex, bool> m_Expandes;
};

PluginDialog::PluginDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::PluginDialog) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    ui->listView->setModel(PluginManager::instance());
    ui->listView->setItemDelegate(new PluginDelegate());
}

PluginDialog::~PluginDialog() {
    delete ui;
}

void PluginDialog::on_closeButton_clicked() {
    accept();
}

void PluginDialog::on_loadButton_clicked() {
    QDir dir = QDir(QDir::currentPath());
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Please select Thunder Engine Mod"),
                                                dir.absolutePath(),
                                                tr("Mods (*.dll *.mod)") );
    if(!path.isEmpty()) {
        PluginManager *model = PluginManager::instance();
        model->loadPlugin(dir.relativeFilePath(path));
    }
}

void PluginDialog::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
