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
#include "pluginmodel.h"

#include <QStringList>

#include <editor.h>
#include <editor/pluginmanager.h>
#include <object.h>

class PluginModelObserver : public Object {
    A_OBJECT(PluginModelObserver, Object, Editor)

    A_METHODS(
        A_SLOT(PluginModelObserver::onListChanged)
    )

public:
    PluginModelObserver() :
            m_model(nullptr) {
    }

    void setModel(PluginModel *model) {
        m_model = model;
    }

private:
    void onListChanged() {
        m_model->onPluginListChanged();
    }

    PluginModel *m_model;
};

PluginModel::PluginModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_observer(new PluginModelObserver),
        m_rowCount(Editor::plugins()->pluginCount()) {
    m_observer->setModel(this);
    Object::connect(Editor::plugins(), _SIGNAL(listChanged()), m_observer, _SLOT(onListChanged()));
}

PluginModel::~PluginModel() {
    delete m_observer;
}

int PluginModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : PLUGIN_BETA + 1;
}

QVariant PluginModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }

    PluginInfo info;
    if(!Editor::plugins()->pluginInfo(index.row(), info)) {
        return QVariant();
    }

    switch(index.column()) {
        case PLUGIN_NAME:        return QString::fromStdString(info.name.data());
        case PLUGIN_DESCRIPTION: return QString::fromStdString(info.description.data());
        case PLUGIN_VERSION:     return QString::fromStdString(info.version.data());
        case PLUGIN_AUTHOR:      return QString::fromStdString(info.author.data());
        case PLUGIN_PATH:        return QString::fromStdString(info.path.data());
        case PLUGIN_ENABLED:     return info.enabled;
        case PLUGIN_TAGS: {
            QStringList tags;
            for(const TString &tag : info.tags) {
                tags.push_back(QString::fromStdString(tag.data()));
            }
            return tags;
        }
        case PLUGIN_BETA:        return info.beta;
        default:                 return QVariant();
    }
}

bool PluginModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid() || index.column() != PLUGIN_ENABLED || role != Qt::EditRole) {
        return false;
    }

    if(!Editor::plugins()->setPluginEnabled(index.row(), value.toBool())) {
        return false;
    }

    emit listChanged();
    return true;
}

int PluginModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : Editor::plugins()->pluginCount();
}

QModelIndex PluginModel::index(int row, int column, const QModelIndex &parent) const {
    if(parent.isValid() || row < 0 || row >= rowCount() || column < 0 || column >= columnCount()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex PluginModel::parent(const QModelIndex &) const {
    return QModelIndex();
}

void PluginModel::onPluginListChanged() {
    int newRowCount = Editor::plugins()->pluginCount();
    if(newRowCount > m_rowCount) {
        beginInsertRows(QModelIndex(), m_rowCount, newRowCount - 1);
        m_rowCount = newRowCount;
        endInsertRows();
    } else if(newRowCount < m_rowCount) {
        beginResetModel();
        m_rowCount = newRowCount;
        endResetModel();
    } else if(newRowCount > 0) {
        emit dataChanged(index(0, 0), index(newRowCount - 1, PLUGIN_BETA));
    }

}
