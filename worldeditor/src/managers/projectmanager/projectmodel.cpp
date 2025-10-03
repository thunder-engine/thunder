#include "projectmodel.h"

#include <QSettings>
#include <QApplication>
#include <QFont>
#include <QDir>

#include <url.h>
#include <file.h>

#include "config.h"

const char *gProjects("launcher.projects");

ProjectModel::ProjectModel() :
        QAbstractListModel() {

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value  = settings.value(gProjects);
    if(value.isValid()) {
        m_list = value.toStringList();
        for(const QString it : m_list) {
            if(!File::exists(it.toStdString())) {
                m_list.removeAll(it);
            }
        }

        for(const QString it : m_list) {
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

    TString path = m_list.at(index.row()).toStdString();

    switch(role) {
        case Qt::DisplayRole: { return Url(path).baseName().data(); }
        case Qt::ToolTipRole:
        case Qt::EditRole: { return path.data(); }
        case Qt::DecorationRole: { return m_iconCache.value(path.data()); }
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
