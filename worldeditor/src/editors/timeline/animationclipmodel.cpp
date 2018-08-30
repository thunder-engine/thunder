#include "animationclipmodel.h"

#include <resources/animationclip.h>
#include <components/animationcontroller.h>
#include <components/actor.h>

#include <iterator>

#include <QColor>

AnimationClipModel::AnimationClipModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_pController(nullptr),
        m_isHighlighted(false) {

}

void AnimationClipModel::setController(AnimationController *controller) {
    m_pController = controller;

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QVariant AnimationClipModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || m_pController == nullptr || m_pController->clip() == nullptr) {
        return QVariant();
    }

    AnimationClip *clip = m_pController->clip();

    auto it = clip->m_Tracks.begin();
    advance(it, index.row());
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            QStringList lst = QString::fromStdString(it->path).split('/');
            QString name    = lst.last();
            uint32_t size   = lst.size();
            if(name.isEmpty()) {
                name    = QString::fromStdString(m_pController->actor().name());
                size    = 0;
            }

            QString spaces;
            for(auto i = 0; i < size; i++) {
                spaces  += "    ";
            }
            return QString("%1%2 : %3").arg(spaces).arg(name).arg(it->property.c_str());
        }
        case Qt::BackgroundColorRole: {
            if(m_isHighlighted && (index == m_HoverIndex)) {
                return QColor(229, 0, 0);
            }
        }
        default: break;
    }

    return QVariant();
}

QVariant AnimationClipModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("");
        }
    }
    return QVariant();
}

int AnimationClipModel::columnCount(const QModelIndex &) const {
    return 1;
}

QModelIndex AnimationClipModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    AnimationClip::TrackList *list  = nullptr;
    if(m_pController && m_pController->clip()) {
        list    = &m_pController->clip()->m_Tracks;
    }
    return createIndex(row, column, list);
}

QModelIndex AnimationClipModel::parent(const QModelIndex &) const {
    return QModelIndex();
}

int AnimationClipModel::rowCount(const QModelIndex &parent) const {
    if(m_pController && m_pController->clip() && !parent.isValid()) {
        return m_pController->clip()->m_Tracks.size();
    }
    return 0;
}

void AnimationClipModel::setHighlighted(const QModelIndex &index) {
    if(index != m_HoverIndex || !m_isHighlighted) {
        if(m_isHighlighted) {
            emit dataChanged(m_HoverIndex, m_HoverIndex);
        }
        m_HoverIndex    = index;
        m_isHighlighted = true;
        emit dataChanged(index, index);

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }

}
