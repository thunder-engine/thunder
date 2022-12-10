#ifndef OBJECTSELECTBROWSER_H
#define OBJECTSELECTBROWSER_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include <object.h>

class ObjectsFilter;
class AssetFilter;

class ProxyModelNoneEntry;

class QTreeView;

namespace Ui {
    class ObjectSelectBrowser;
}

class ObjectSelectBrowser : public QWidget {
    Q_OBJECT

public:
    explicit ObjectSelectBrowser(QWidget *parent = nullptr);
    ~ObjectSelectBrowser();

    void setTypeFilter(const QString &filter);

    Object *findObject(uint32_t id);

signals:
    void componentSelected(Object *object);
    void assetSelected(QString asset);

public slots:
    void onObjectSelected(Object *object);
    void onAssetSelected(const QString &path);

    void onSetRootObject(Object *object);

private slots:
    void onModelUpdated();

    void on_lineEdit_textChanged(const QString &arg1);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_listView_doubleClicked(const QModelIndex &index);

private:
    void expandToIndex(const QModelIndex &index, QTreeView *view);

private:
    Ui::ObjectSelectBrowser *ui;

    QString m_asset;
    Object *m_object;

    ObjectsFilter *m_componentProxy;
    AssetFilter *m_contentProxy;

    ProxyModelNoneEntry *m_contentNone;
};

class ProxyModelNoneEntry : public QSortFilterProxyModel {
    Q_OBJECT

public:
    ProxyModelNoneEntry(QObject *parent) :
        QSortFilterProxyModel(parent) {

    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        if(!parent.isValid()) {
            return QSortFilterProxyModel::rowCount()+1;
        }
        return QSortFilterProxyModel::rowCount();
    }

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const {
        if(!sourceIndex.isValid()) {
            return QModelIndex();
        } else if(sourceIndex.parent().isValid()) {
            return QModelIndex();
        }
        return createIndex(sourceIndex.row()+1, sourceIndex.column());
    }

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const {
        if(!proxyIndex.isValid()) {
            return QModelIndex();
        } else if(proxyIndex.row() == 0) {
            return QModelIndex();
        }
        return sourceModel()->index(proxyIndex.row()-1, proxyIndex.column());
    }

    QVariant data(const QModelIndex &index, int role) const {
        if(!index.isValid()) {
            return QVariant();
        }

        if(index.row() == 0) {
            switch(role) {
            case Qt::EditRole:
            case Qt::ToolTipRole:
            case Qt::DisplayRole: return "None";
            case Qt::SizeHintRole: return QSize(80, 80);
            default: return QVariant();
            }
        }
        return QSortFilterProxyModel::data(createIndex(index.row(), index.column(), index.internalPointer()), role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const {
        if(!index.isValid()) {
            return Qt::NoItemFlags;
        }

        if(index.row() == 0) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
        return QSortFilterProxyModel::flags(createIndex(index.row(), index.column()));
    }

    QModelIndex index(int row, int column, const QModelIndex &parent) const {
        if(row > rowCount()) {
            return QModelIndex();
        }
        return createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex &child) const {
        Q_UNUSED(child)
        return QModelIndex();
    }
};

#endif // OBJECTSELECTBROWSER_H
