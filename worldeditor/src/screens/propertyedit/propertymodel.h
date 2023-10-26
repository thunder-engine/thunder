#ifndef PROPERTYMODEL_H
#define PROPERTYMODEL_H

#include <QMap>

#include "propertyeditor.h"

#include "screens/baseobjectmodel/baseobjectmodel.h"

class Property;

class PropertyModel : public BaseObjectModel {
	Q_OBJECT

public:
    explicit PropertyModel(QObject* parent = nullptr);

    ~PropertyModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addItem(QObject *propertyObject, const QString &propertyName = QString(), QObject *parent = nullptr);

    void updateItem(QObject *propertyObject);

    void clear();

private:
    void updateDynamicProperties(Property *parent, QObject *propertyObject);

};

QString fromCamelCase(const QString &s);

#endif
