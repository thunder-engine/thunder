#include "projectmodel.h"

#include <QSettings>

#include "config.h"

const char *gProjects("launcher.projects");

ProjectModel::ProjectModel() :
        QAbstractListModel() {

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value  = settings.value(gProjects);
    if(value.isValid()) {
        m_list = value.toStringList();
        for(const QString it : m_list) {
            if(!QFileInfo::exists(it)) {
                m_list.removeAll(it);
            }
        }

        for(const QString it : m_list) {
            QFileInfo info(it);
            QFileInfo icon(info.absolutePath() + "/cache/thumbnails/auto.png");

            QImage image(":/Images/icons/thunderlight.svg");
            if(icon.isReadable()) {
                image = QImage(icon.absoluteFilePath());
            }

            image = image.scaledToWidth(90);

            m_iconCache[info.absoluteFilePath()] = image;
        }
    }
}

ProjectModel::~ProjectModel() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gProjects, m_list);
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return m_list.size();
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    QFileInfo info(m_list.at(index.row()));
    switch(role) {
        case Qt::DisplayRole: { return info.baseName(); }
        case Qt::EditRole: { return info.absoluteFilePath(); }
        case Qt::DecorationRole: { return m_iconCache.value(info.absoluteFilePath()); }
        default: break;
    }
    return QVariant();
}

void ProjectModel::addProject(const QString &path) {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value(gProjects);
    if(value.isValid()) {
        QStringList list = value.toStringList();
        list << QDir::fromNativeSeparators(path);
        list.removeDuplicates();

        settings.setValue(gProjects, list);
    }
}
