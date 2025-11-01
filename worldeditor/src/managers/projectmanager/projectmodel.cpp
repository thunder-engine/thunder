#include "projectmodel.h"

#include <QSettings>
#include <QApplication>
#include <QFont>

#include <url.h>
#include <file.h>

const char *gProjects("launcher.projects");

StringList ProjectModel::s_list;

ProjectModel::ProjectModel() :
        QAbstractListModel() {

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value(gProjects);
    if(value.isValid()) {
        for(auto &it : value.toStringList()) {
            if(File::exists(it.toStdString())) {
                s_list.push_back(it.toStdString());

                TString icon(Url(it.toStdString()).absoluteDir() + "/cache/thumbnails/auto.png");

                QImage image(":/Images/icons/thunderlight.svg");
                if(File::exists(icon)) {
                    image = QImage(icon.data());
                }
                image = image.scaledToWidth(64);

                m_iconCache[it] = image;
            }
        }
    }
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return s_list.size();
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    TString path = *std::next(s_list.begin(), index.row());

    switch(role) {
        case Qt::DisplayRole: { return Url(path).baseName().data(); }
        case Qt::ToolTipRole:
        case Qt::EditRole: { return path.data(); }
        case Qt::DecorationRole: { return m_iconCache[path.data()]; }
        case Qt::FontRole: {
            QFont font = QApplication::font("QTreeView");
            font.setBold(true);
            font.setPointSize(10);
            return font;
        }
        default: break;
    }
    return QVariant();
}

StringList removeDuplicatesManual(const StringList &original) {
    StringList unique;
    for(const TString &s : original) {
        bool found = false;
        for(const TString &us : unique) {
            if(s == us) {
                found = true;
                break;
            }
        }
        if(!found) {
            unique.push_back(s);
        }
    }
    return unique;
}

void ProjectModel::addProject(const TString &path) {
    s_list.push_front(path);

    QStringList list;
    for(auto &it : removeDuplicatesManual(s_list)) {
        list << it.data();
    }
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gProjects, list);

}
