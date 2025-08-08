#include "nextmodel.h"

#include <QFont>
#include <QPalette>
#include <QApplication>
#include <QRegularExpression>

#include "property.h"

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

void NextModel::addItem(Object *propertyObject) {
    const MetaObject *metaObject = propertyObject->metaObject();

    Property *propertyItem = static_cast<Property *>(m_rootItem);

    int count = metaObject->propertyCount();
    if(count) {
        TString name = metaObject->name();

        propertyItem = new Property(name, static_cast<Property *>(m_rootItem), true);
        propertyItem->setPropertyObject(propertyObject);

        connect(propertyItem, &Property::propertyChanged, this, &NextModel::propertyChanged);

        for(int i = 0; i < count; i++) {
            MetaProperty property = metaObject->property(i);

            if(!TString(property.name()).toLower().contains("enable")) {
                uint32_t type = property.read(propertyObject).type();
                if(type < MetaType::QUATERNION || type >= MetaType::OBJECT) {
                    Property *p = new Property(property.name(), (propertyItem) ? propertyItem : static_cast<Property *>(m_rootItem), false);
                    p->setPropertyObject(propertyObject);

                    const char *annotation = property.table()->annotation;
                    if(annotation) {
                        p->setEditorHints(annotation);
                    }

                    connect(p, &Property::propertyChanged, this, &NextModel::propertyChanged);
                }
            }
        }
    }

    if(propertyItem) {
        updateDynamicProperties(propertyItem, propertyObject);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void NextModel::updateDynamicProperties(Property *parent, Object *propertyObject) {
    // Get dynamic property names
    auto dynamicProperties = propertyObject->dynamicPropertyNames();

    StringList dynamicPropertiesFiltered;
    // Remove invalid properites and those we don't want to add
    for(auto it : dynamicProperties) {
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
        QStringList list = QString(dynProp.data()).split('/');

        Property *s = it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                QString path = list.mid(0, i + 1).join('/');
                Property *child = it->findChild<Property *>(path);
                if(child) {
                    it = child;
                } else {
                    p = new Property(path.toStdString(), it, parent == m_rootItem);
                    p->setPropertyObject(propertyObject);

                    it = p;
                }
            } else if(!list[i].isEmpty()) {
                p = new Property(dynProp, it, false);
                p->setPropertyObject(propertyObject);
                p->setEditorHints(propertyObject->dynamicPropertyInfo(dynProp.data()));

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
    Property *item = static_cast<Property* >(index.internalPointer());
    switch(role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
        case Qt::EditRole: {
            if(index.column() == 0) {
                return fromCamelCase(item->name().replace('_', ' '));
            }
            if(index.column() == 1 && (item->editor() == nullptr || role == Qt::EditRole)) {
                return item->value(role);
            }
        } break;
        case Qt::BackgroundRole: {
            if(item->isRoot()) {
                return QApplication::palette("QTreeView").brush(QPalette::Normal, QPalette::Button).color();
            }
            if(index.column() == 0) {
                return QColor("#ff0000ff");
            }
        } break;
        case Qt::DecorationRole: {
            if(index.column() == 1) {
                return item->value(role);
            }
        } break;
        case Qt::FontRole: {
            if(item->isRoot()) {
                QFont font = QApplication::font("QTreeView");
                font.setBold(true);
                font.setPointSize(font.pointSizeF() + 2);
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

    return QVariant();
}

// edit methods
bool NextModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid()) {
        return false;
    }
    Property *item = static_cast<Property *>(index.internalPointer());
    if(role == Qt::EditRole) {
        item->setValue(value);
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
    Property *item = static_cast<Property *>(index.internalPointer());
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
    delete m_rootItem;
    m_rootItem = new Property("Root", nullptr, true);

    beginResetModel();
    endResetModel();
}

