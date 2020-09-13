#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>
#include <QAbstractItemModel>

#include "builder.h"

#include "resources/angelscript.h"

class asSMessageInfo;
class asIScriptEngine;
class asITypeInfo;
class asIScriptFunction;

class AngelSystem;

class AngelSerial : public AngelScript {
public:
    VariantMap saveUserData() const;
protected:
    friend class AngelBuilder;

};

class AngelClassItem {
public:
    explicit AngelClassItem(const QVector<QVariant> &data, AngelClassItem *parentItem = nullptr);
    ~AngelClassItem();

    void appendChild(AngelClassItem *child);

    AngelClassItem *child(int row);
    int childCount() const;
    QVariant data(int column) const;
    int row() const;
    AngelClassItem *parentItem();

private:
    QList<AngelClassItem*> m_childItems;
    QVector<QVariant> m_itemData;
    AngelClassItem *m_parentItem;
};

class AngelClassMapModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum AngelItem {
        Module = 0,
        Class,
        Method,
        Property,
        Enum
    };

public:
    AngelClassMapModel(asIScriptEngine *engine);

    void update();

private:
    void exportType(asITypeInfo *info, AngelItem type = Class);
    QStringList exportParams(asIScriptFunction *method);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    asIScriptEngine *m_pEngine;

    AngelClassItem *m_pRootItem;
};

class AngelBuilder : public IBuilder {
    Q_OBJECT
public:
    AngelBuilder(AngelSystem *system);

protected:
    bool buildProject() Q_DECL_OVERRIDE;

    QString builderVersion() Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE { return {"as"}; }
    uint32_t type() const Q_DECL_OVERRIDE { return MetaType::type<AngelScript *>(); }

    QAbstractItemModel *classMap() const Q_DECL_OVERRIDE;

    uint8_t convertFile(IConverterSettings *settings) Q_DECL_OVERRIDE;

    const QString persistentAsset() const Q_DECL_OVERRIDE;
    const QString persistentUUID() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Angel_Behaviour.as"; }

    static void messageCallback(const asSMessageInfo *msg, void *param);

    AngelSystem *m_pSystem;

    asIScriptEngine *m_pScriptEngine;

    QString m_Destination;

    AngelClassMapModel *m_pClassModel;

};

#endif // AUDIOCONVERTER_H
