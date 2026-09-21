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
#include "nextmodel.h"

#include <QFont>
#include <QPalette>
#include <QApplication>
#include <QRegularExpression>

#include "property.h"
#include <editor/propertyedit.h>

QString fromCamelCase(const TString &s) {
    static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

    QString result = s.data();
    result.replace(regExp1, "\\1 \\2");
    result.replace(regExp2, "\\1 \\2");

    result[0] = result[0].toUpper();

    return result;
}

NextModel::NextModel(QObject *parent):
        BaseObjectModel(parent) {
}

NextModel::~NextModel() {
    delete m_rootItem;
}

void NextModel::addObject(Object *propertyObject) {
    addObjects({propertyObject});
}

void NextModel::addObjects(const Object::ObjectList &propertyObjects) {
    if(propertyObjects.empty()) {
        return;
    }

    Object *propertyObject = propertyObjects.front();
    const MetaObject *metaObject = propertyObject->metaObject();

    TString name = propertyObject->typeName();
    if(name.isEmpty()) {
        name = metaObject->name();
    }

    Property *propertyItem = new Property(name, static_cast<Property *>(m_rootItem), true);
    propertyItem->setPropertyObjects(propertyObjects);
    addItem(propertyItem);

    int count = metaObject->propertyCount();
    if(count) {
        connect(propertyItem, &Property::propertyChanged, this, &NextModel::propertyChanged);

        for(int i = 0; i < count; i++) {
            MetaProperty property = metaObject->property(i);

            if(!TString(property.name()).toLower().contains("enable")) {
                bool presentOnAllObjects = true;
                auto object = propertyObjects.begin();
                ++object;
                for(; object != propertyObjects.end(); ++object) {
                    if((*object)->metaObject()->indexOfProperty(property.name()) < 0) {
                        presentOnAllObjects = false;
                        break;
                    }
                }
                if(!presentOnAllObjects) {
                    continue;
                }

                uint32_t type = property.read(propertyObject).type();
                if(type < MetaType::QUATERNION || type >= MetaType::OBJECT) {
                    Property *p = new Property(property.name(), (propertyItem) ? propertyItem : static_cast<Property *>(m_rootItem), false);
                    p->setPropertyObjects(propertyObjects);
                    addItem(p);

                    const char *annotation = property.table()->annotation;
                    if(annotation) {
                        p->setEditorHints(annotation);
                    }

                    connect(p, &Property::propertyChanged, this, &NextModel::propertyChanged);
                }
            }
        }
    }

    updateDynamicProperties(propertyItem, propertyObject);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void NextModel::updateDynamicProperties(Property *parent, Object *propertyObject) {
    // Get dynamic property names
    auto dynamicProperties = propertyObject->dynamicPropertyNames();

    StringList dynamicPropertiesFiltered;
    // Remove invalid properites and those we don't want to add
    for(auto &it : dynamicProperties) {
        // Skip user defined hidden properties starting with _
        if(it.front() != '_') {
            dynamicPropertiesFiltered.push_back(it);
        }
    }

    if(dynamicPropertiesFiltered.empty()) {
        return;
    }

    Property *it = parent;
    // Add properties left in the list

    for(const TString &dynProp : dynamicPropertiesFiltered) {
        uint32_t type = propertyObject->property(dynProp.data()).type();
        if(type == MetaType::VARIANTMAP) {
            continue;
        }

        StringList list = TString(dynProp).split('/');

        Property *s = it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                TString path = TString::join(StringList(list.begin(), std::next(list.begin(), i + 1)), "/");
                Property *child = it->findChild<Property *>(path.data());
                if(child) {
                    it = child;
                } else {
                    p = new Property(path.toStdString(), it, parent == m_rootItem);
                    p->setPropertyObject(propertyObject);
                    addItem(p);

                    it = p;
                }
            } else if(!std::next(list.begin(), i)->isEmpty()) {
                p = new Property(dynProp, it, false);
                p->setPropertyObject(propertyObject);
                p->setEditorHints(propertyObject->dynamicPropertyInfo(dynProp.data()));
                addItem(p);

                connect(p, &Property::propertyChanged, this, &NextModel::propertyChanged);

                p->setProperty("__Dynamic", true);
            }
        }
        it = s;
    }
}

int NextModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant NextModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    Property *item = static_cast<Property *>(getObject(index));
    if(item) {
        switch(role) {
            case Qt::ToolTipRole:
            case Qt::DisplayRole:
            case Qt::EditRole: {
                if(index.column() == 0) {
                    return fromCamelCase(item->name().replace('_', ' '));
                }
            } break;
            case Qt::BackgroundRole: {
                if(item->isRoot()) {
                    return QApplication::palette("QTreeView").brush(QPalette::Normal, QPalette::Button).color();
                }
                if(index.column() == 0) {
                    return QColor(0, 0, 255);
                }
            } break;
            case Qt::FontRole: {
                if(item->isRoot()) {
                    QFont font = QApplication::font("QTreeView");
                    font.setBold(true);
                    return font;
                }
            } break;
            case Qt::SizeHintRole: {
                return QSize(1, 26);
            }
            case Qt::CheckStateRole: {
                if(index.column() == 0 && item->isCheckable()) {
                    return item->isChecked() ? Qt::Checked : Qt::Unchecked;
                }
            } break;
            default: break;
        }
    }

    return QVariant();
}

// edit methods
bool NextModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid()) {
        return false;
    }
    Property *item = static_cast<Property *>(getObject(index));
    if(role == Qt::EditRole) {
        emit dataChanged(index, index);
        return true;
    } else if(role == Qt::CheckStateRole) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        item->setChecked(state == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags NextModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    Property *item = static_cast<Property *>(getObject(index));
    // only allow change of value attribute

    Qt::ItemFlags result;
    if(item->isRoot()) {
        result |= Qt::ItemIsEnabled;
    } else if(item->isReadOnly()) {
        result |= Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    } else {
        result |= Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

        if(index.column() == 1) {
            result |= Qt::ItemIsEditable;
        }
    }

    if(index.column() == 0 && item->isCheckable()) {
        result |= Qt::ItemIsUserCheckable;
    }

    return result;
}

QVariant NextModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Property");
            case 1: return tr("Value");
        }
    }
    return QVariant();
}

void NextModel::clear() {
    BaseObjectModel::clear();

    delete m_rootItem;
    m_rootItem = new Property("Root", nullptr, true);
    addItem(m_rootItem);

    beginResetModel();
    endResetModel();
}

