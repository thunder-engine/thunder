#ifndef OBJECTHIERARCHYMODEL_H
#define OBJECTHIERARCHYMODEL_H

#include <QAbstractItemModel>
#include <QPixmap>

class Object;

class ObjectHierarchyModel : public QAbstractItemModel {
    Q_OBJECT

public:
    ObjectHierarchyModel(QObject *parent);

    void setRoot(Object *root);

    Object *root() const { return m_rootItem; }

    Object *findObject(const uint32_t uuid, Object *parent = nullptr);

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

    QPixmap m_visible;
    QPixmap m_invisible;

    QPixmap m_select;
    QPixmap m_selectDisable;

    QPixmap m_prefab;
    QPixmap m_actor;

};

#endif // OBJECTHIERARCHYMODEL_H
