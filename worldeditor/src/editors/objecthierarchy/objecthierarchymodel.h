#ifndef OBJECTHIERARCHYMODEL_H
#define OBJECTHIERARCHYMODEL_H

#include <QAbstractItemModel>
#include <QPixmap>

class Object;

class ObjectHierarchyModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ObjectHierarchyModel(QObject *parent);

    void setRoot(Object *scene);

    Object *root() const { return m_rootItem; }

    Object *findObject(const uint32_t uuid, Object *parent = nullptr);

private:
    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

protected:
    Object *m_rootItem;

    QPixmap m_Visible;
    QPixmap m_Invisible;

    QPixmap m_Select;
    QPixmap m_SelectDisable;

    QPixmap m_Prefab;
    QPixmap m_Actor;
};

#endif // OBJECTHIERARCHYMODEL_H
