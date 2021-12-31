#include "assetbrowser.h"
#include "ui_assetbrowser.h"

#include <QDir>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#include <global.h>

#include "assetlist.h"
#include "assetlist.h"

#include "assetmanager.h"
#include <editor/projectmanager.h>

class AssetFilter : public QSortFilterProxyModel {
public:
    typedef QList<int32_t> TypeList;

    explicit AssetFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
        sort(0);
    }

    void setContentType(const QString &type) {
        m_Type = type;
        invalidate();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;
        if(!m_Type.isEmpty()) {
            result = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        if(m_Type == sourceModel()->data(index).toString()) {
            return true;
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        return (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) &&
               (filterRegExp().isEmpty() || sourceModel()->data(index).toBool()));
    }

    QString m_Type;
};

class AssetItemDeligate : public QStyledItemDelegate  {
public:
    explicit AssetItemDeligate(QObject *parent = nullptr) :
            QStyledItemDelegate(parent) {
    }

private:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        QVariant value = index.data(Qt::DecorationRole);
        switch(value.type()) {
            case QVariant::Image: {
                QImage image = value.value<QImage>();
                if(!image.isNull()) {
                    QSize origin = image.size();
                    image = image.scaled(origin.width(), origin.height(),
                                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    option->icon = QIcon(QPixmap::fromImage(image));
                    option->decorationSize = image.size();
                }
            } break;
            default: break;
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
        return new QLineEdit(parent);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
        QRect r = editor->geometry();

        r.setX(option.rect.x());
        r.setWidth(option.rect.width());
        editor->setGeometry(r);

        QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
        if(e) {
            e->setAlignment(Qt::AlignHCenter);
            e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const {
        int32_t width = editor->width();
        QStyledItemDelegate::setEditorData(editor, index);
        editor->setFixedWidth(width);
    }
};

AssetBrowser::AssetBrowser(QWidget* parent) :
        QWidget(parent),
        ui(new Ui::AssetBrowser) {

    ui->setupUi(this);

    m_pContentDeligate = new AssetItemDeligate();
    m_pContentProxy = new AssetFilter(this);
    m_pContentProxy->setSourceModel(AssetList::instance());

    ui->assetList->setItemDelegate(m_pContentDeligate);
    ui->assetList->setModel(m_pContentProxy);

    connect(AssetList::instance(), &AssetList::layoutChanged, this, &AssetBrowser::onModelUpdated);
}

AssetBrowser::~AssetBrowser() {
    delete ui;
}

void AssetBrowser::onModelUpdated() {
    setSelected(m_Resource);
    m_pContentProxy->invalidate();
}

void AssetBrowser::filterByType(const QString &type) {
    m_pContentProxy->setContentType(type);
}

void AssetBrowser::setSelected(const QString &resource) {
    m_Resource = resource;
    ui->assetList->setCurrentIndex( m_pContentProxy->mapFromSource(AssetList::instance()->findResource(m_Resource)) );
}

QImage AssetBrowser::icon(const QString &resource) const {
    return AssetList::instance()->icon( AssetList::instance()->findResource(resource) );
}

void AssetBrowser::on_findContent_textChanged(const QString &arg1) {
    m_pContentProxy->setFilterFixedString(arg1);
}

void AssetBrowser::on_assetList_clicked(const QModelIndex &index) {
    QModelIndex origin = m_pContentProxy->mapToSource(index);

    QString source = AssetList::instance()->path(origin);
    emit assetSelected(AssetManager::instance()->pathToGuid(source.toStdString()).c_str());
}

void AssetBrowser::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
