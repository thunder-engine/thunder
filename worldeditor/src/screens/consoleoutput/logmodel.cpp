#include "logmodel.h"

#include <QIcon>
#include <log.h>

LogModel::LogModel() :
        QAbstractItemModel(),
        m_Error(QIcon(":/Images/message/Error.png")),
        m_Warning(QIcon(":/Images/message/Warning.png")),
        m_Info(QIcon(":/Images/message/Information.png")) {

}

void LogModel::addRecord(uint8_t type, const QString &str) {
    m_Records.append(str);
    m_Types.append(type);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void LogModel::clear() {
    m_Records.clear();
    m_Types.clear();

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

int LogModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant LogModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
        case Qt::DisplayRole: return m_Records.at(index.row());
        case Qt::DecorationRole: {
            switch(m_Types.at(index.row())) {
                case Log::INF: return m_Info;
                case Log::WRN: return m_Warning;
                default: return m_Error;
            }
        } break;
        default: break;
    }

    return QVariant();
}

int LogModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/ ) const {
    return m_Records.size();
}

QModelIndex LogModel::index(int row, int column, const QModelIndex &parent) const {
    return createIndex(row, column);
}

QModelIndex LogModel::parent(const QModelIndex &index) const {
    return QModelIndex();
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
