#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>
#include <QAbstractItemModel>

#include <codebuilder.h>

#include "resources/angelscript.h"

class asSMessageInfo;
class asIScriptEngine;
class asITypeInfo;
class asIScriptFunction;

class AngelSystem;

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

class AngelScriptImportSettings : public BuilderSettings {
public:
    explicit AngelScriptImportSettings(CodeBuilder *builder);
};

class AngelBuilder : public CodeBuilder {
    Q_OBJECT
public:
    AngelBuilder(AngelSystem *system);
    ~AngelBuilder() override;

protected:
    void init() override;

    bool isNative() const override;

    bool buildProject() override;

    QString builderVersion() override;

    QStringList suffixes() const override { return {"as"}; }
    QAbstractItemModel *classMap() const override;

    ReturnCode convertFile(AssetConverterSettings *settings) override;

    AssetConverterSettings *createSettings() override;

    const QString persistentAsset() const override;
    const QString persistentUUID() const override;

    QString templatePath() const override { return ":/templates/AngelBehaviour.as"; }

    static void messageCallback(const asSMessageInfo *msg, void *param);

    AngelSystem *m_system;

    asIScriptEngine *m_scriptEngine;

    QString m_Destination;

    AngelClassMapModel *m_classModel;

};

#endif // AUDIOCONVERTER_H
