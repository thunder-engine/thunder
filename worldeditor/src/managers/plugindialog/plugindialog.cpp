#include <QFileDialog>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <QProcess>
#include <QSortFilterProxyModel>

#include "ui_plugindialog.h"

#include "plugindialog.h"

#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>

#define ROW_HEIGHT 24
#define ICON_SIZE 16

class PluginDelegate : public QStyledItemDelegate {
private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        const QPalette &palette(opt.palette);
        QRect rect(opt.rect);

        painter->save();
        painter->setClipping(true);
        painter->setClipRect(rect);
        painter->setFont(opt.font);

        painter->setPen(palette.color(QPalette::Shadow));

        QRect border(rect);
        border.adjust(1, 1,-1,-1);
        painter->drawRoundedRect(border, 4, 4);

        rect.setHeight(ROW_HEIGHT);
        rect.setLeft(ICON_SIZE * 3);

        painter->setPen(palette.windowText().color());
        painter->drawText(rect, Qt::TextSingleLine | Qt::AlignVCenter, opt.text);

        QRect icon(2, rect.y() + (ROW_HEIGHT - ICON_SIZE) / 2, ICON_SIZE, ICON_SIZE);

        const QSortFilterProxyModel *filter = static_cast<const QSortFilterProxyModel *>(index.model());
        QAbstractItemModel *m = filter->sourceModel();

        bool expanded = m_expandes.value(index, false);
        if(expanded) {
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
        static QImage right(":/Style/styles/dark/icons/arrow-right.png");
        static QImage down(":/Style/styles/dark/icons/arrow-down.png");

        painter->drawImage(icon, expanded ? down : right);

        // Draw checkbox
        icon.moveLeft(ICON_SIZE + 4);

        static QBrush checkBrush(QColor("#525252"));

        painter->setPen(Qt::NoPen);
        painter->setBrush(checkBrush);
        painter->drawRoundedRect(icon, 3, 3);

        if(m->data(m->index(index.row(), PluginManager::PLUGIN_ENABLED)).toBool()) {
            static QImage check(":/Style/styles/dark/icons/check.png");
            painter->drawImage(icon, check);
        }

        // Draw tags
        static QBrush tagBrush(QColor("#0277bd"));
        QFontMetrics metrics = painter->fontMetrics();

        icon.moveLeft(ICON_SIZE * 4 + metrics.boundingRect(opt.text).width());

        for(auto &it : m->data(m->index(index.row(), PluginManager::PLUGIN_TAGS)).toStringList()) {
            icon.setWidth(metrics.boundingRect(it).width() + ICON_SIZE * 2);

            painter->setPen(Qt::NoPen);
            painter->setBrush(tagBrush);
            painter->drawRoundedRect(icon, ICON_SIZE / 2, ICON_SIZE / 2);

            painter->setPen(palette.windowText().color());
            painter->drawText(icon, Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignHCenter, it);

            icon.moveLeft(icon.x() + icon.width() + ICON_SIZE);
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);

        size.setHeight(m_expandes.value(index, false) ? (PluginManager::PLUGIN_LAST * ROW_HEIGHT) : ROW_HEIGHT);
        return size;
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        QRect rect(option.rect);

        QMouseEvent *ev = dynamic_cast<QMouseEvent *>(event);
        if(ev && ev->type() == QEvent::MouseButtonPress && ev->button() == Qt::LeftButton) {
            QRect arrow(0, rect.y() + 5, ICON_SIZE, ICON_SIZE);
            if(arrow.contains(ev->pos())) {
                m_expandes[index] = !m_expandes[index];
                emit model->layoutAboutToBeChanged();
                emit model->layoutChanged();
            }
            QRect check(ICON_SIZE + 4, rect.y() + 5, ICON_SIZE, ICON_SIZE);
            if(check.contains(ev->pos())) {
                const QSortFilterProxyModel *filter = static_cast<const QSortFilterProxyModel *>(index.model());
                QAbstractItemModel *m = filter->sourceModel();

                QModelIndex origin = m->index(filter->mapToSource(index).row(), PluginManager::PLUGIN_ENABLED);
                m->setData(origin, !m->data(origin).toBool());

                emit model->layoutAboutToBeChanged();
                emit model->layoutChanged();
            }
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    QMap<QModelIndex, bool> m_expandes;

};

PluginDialog::PluginDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::PluginDialog),
        m_filter(new QSortFilterProxyModel) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    m_filter->setSourceModel(PluginManager::instance());

    ui->listView->setModel(m_filter);
    ui->listView->setItemDelegate(new PluginDelegate());

    ui->notification->setProperty("notification", true);
    ui->notification->setVisible(false);

    connect(PluginManager::instance(), &PluginManager::listChanged, ui->notification, &QWidget::show);
}

PluginDialog::~PluginDialog() {
    delete ui;
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

void PluginDialog::on_restartButton_clicked() {
    qApp->quit();

    QProcess::startDetached(qApp->arguments().first(), {ProjectSettings::instance()->projectPath()});
}

void PluginDialog::on_lineEdit_textChanged(const QString &arg1) {
    m_filter->setFilterFixedString(arg1);
}

