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
#ifndef PLUGINMODEL_H
#define PLUGINMODEL_H

#include <QAbstractItemModel>

class PluginModelObserver;

class PluginModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum {
        PLUGIN_NAME,
        PLUGIN_DESCRIPTION,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_PATH,
        PLUGIN_LAST,
        PLUGIN_ENABLED,
        PLUGIN_TAGS,
        PLUGIN_BETA
    };

public:
    explicit PluginModel(QObject *parent = nullptr);
    ~PluginModel() override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

signals:
    void listChanged();

private:
    friend class PluginModelObserver;

    void onPluginListChanged();

    PluginModelObserver *m_observer;
    int m_rowCount;
};

#endif // PLUGINMODEL_H
