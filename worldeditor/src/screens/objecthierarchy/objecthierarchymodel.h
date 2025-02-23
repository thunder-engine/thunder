#ifndef OBJECTHIERARCHYMODEL_H
#define OBJECTHIERARCHYMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QPixmap>

#include <invalid.h>
#include <component.h>

class Object;

class ObjectHierarchyModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ObjectHierarchyModel(QObject *parent);

    Object *root() const;
    void setRoot(Object *root);

    void showNone();

private:
    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    Object *m_rootItem;

    bool m_showNone;

    QPixmap m_visible;
    QPixmap m_invisible;

    QPixmap m_select;
    QPixmap m_selectDisable;

    QPixmap m_prefab;
    QPixmap m_actor;

};

class ObjectsFilter : public QSortFilterProxyModel {
public:
    explicit ObjectsFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
    }

    void setClassType(const QString &filter) {
        m_filter = filter;
        invalidate();
    }

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
        QSortFilterProxyModel::sort(column, order);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;

        ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(sourceModel());

        Object *object = Engine::findObject(sourceModel()->index(sourceRow, 1, sourceParent).internalId());

        if(!m_filter.isEmpty()) {
            result &= checkClassTypeFilter(sourceRow, sourceParent);
        }
        result &= (dynamic_cast<Component*>(object) == nullptr);
        result &= (dynamic_cast<class Invalid*>(object) == nullptr);

        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkClassTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 1, sourceParent);
        QString type = sourceModel()->data(index).toString();
        if(!m_filter.contains(type)) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkClassTypeFilter(i, index)) {
                    return true;
                }
            }
            return false;
        }
        return true;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 0, sourceParent);
        if(!filterRegularExpression().isValid() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkNameFilter(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegularExpression());
        }
        return true;
    }

    QString m_filter;

};

struct ObjectData {
    std::string type;
    Scene *scene = nullptr;
    Actor *actor = nullptr;
    Component *component = nullptr;
};
Q_DECLARE_METATYPE(ObjectData)

#endif // OBJECTHIERARCHYMODEL_H
