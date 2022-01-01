#include "projectmodel.h"

#include <QSettings>
#include <QImage>

#include "config.h"

const char *gProjects("launcher.projects");

ProjectModel::ProjectModel() :
        QAbstractListModel() {

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value  = settings.value(gProjects);
    if(value.isValid()) {
        m_List = value.toStringList();
        foreach(const QString it, m_List) {
            if(!QFileInfo::exists(it)) {
                m_List.removeAll(it);
            }
        }
    }
}

ProjectModel::~ProjectModel() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gProjects, m_List);
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return m_List.size();
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    QFileInfo info( m_List.at(index.row()) );
    switch(role) {
        case NameRole: { return info.baseName(); }
        case PathRole: { return info.absoluteFilePath(); }
        case DirRole:  { return info.absolutePath(); }
        case IconRole: {
            QFileInfo icon(info.absolutePath() + "/cache/thumbnails/auto.png");
            if(icon.isReadable()) {
                return "file:///" + icon.absoluteFilePath();
            }
            return "qrc:/Images/icons/thunderlight.svg";
        }
        default: break;
    }
    return QVariant();
}

QHash<int, QByteArray> ProjectModel::roleNames() const {
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[DirRole]  = "dir";
    roles[IconRole] = "icon";

    return roles;
}

void ProjectModel::addProject(const QString &path) {
    m_List.push_front(QDir::fromNativeSeparators(path));
    m_List.removeDuplicates();
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gProjects, m_List);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
