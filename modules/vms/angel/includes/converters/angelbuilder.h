/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>
#include <QAbstractItemModel>

#include <editor/codebuilder.h>

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
    AngelClassMapModel();

    void update(asIScriptEngine *engine);

private:
    void exportType(asITypeInfo *info, AngelItem type = Class);
    QStringList exportParams(asIScriptFunction *method);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    AngelClassItem *m_pRootItem;
};

class AngelScriptImportSettings : public BuilderSettings {
public:
    explicit AngelScriptImportSettings(CodeBuilder *builder);

    StringList typeNames() const override;

};

class AngelBuilder : public CodeBuilder {
public:
    AngelBuilder(AngelSystem *system);
    ~AngelBuilder() override;

protected:
    void init() override;

    bool buildProject() override;

    TString persistentName() const override;
    TString persistentAsset() const override;

    StringList suffixes() const override { return {"as"}; }
    QAbstractItemModel *classMap() const override;

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/templates/AngelBehaviour.as"; }

    static void messageCallback(const asSMessageInfo *msg, void *param);

    AngelSystem *m_system;

    asIScriptEngine *m_scriptEngine;

    AngelClassMapModel *m_classModel;

};

#endif // AUDIOCONVERTER_H
