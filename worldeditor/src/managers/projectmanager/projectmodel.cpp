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
#include "projectmodel.h"

#include <QSettings>
#include <QApplication>
#include <QFont>
#include <QDir>
#include <QFile>
#include <QSaveFile>

#include <url.h>
#include <file.h>
#include <json.h>
#include <log.h>
#include <standardpaths.h>

#include <algorithm>

const char *gProjects("launcher.projects");
namespace {
    const char *gRecentProjectsFile("recent_projects.json");

    void syncRecentProjectsFile(const TString &preferredProject) {
        QSettings settings(COMPANY_NAME, EDITOR_NAME);
        settings.sync();
        if(settings.status() != QSettings::NoError) {
            aWarning() << "Unable to read the World Editor project history from settings.";
        }
        const QStringList legacyProjects = settings.value(gProjects).toStringList();

        const TString configDirectory = StandardPaths::appendOrganizationAndApp(
                    StandardPaths::writableLocation(StandardPaths::ConfigLocation),
                    COMPANY_NAME,
                    EDITOR_NAME);
        if(configDirectory.isEmpty()) {
            aWarning() << "Unable to determine the application configuration directory.";
            return;
        }

        const QString directory = QString::fromUtf8(configDirectory.data());
        if(!QDir().mkpath(directory)) {
            aWarning() << "Unable to create the application configuration directory:" << configDirectory;
            return;
        }
        const QString recentProjectsPath = QDir(directory).filePath(gRecentProjectsFile);

        StringList projects;
        auto appendProject = [&projects](const TString &path) {
            if(!path.isEmpty() && File::exists(path) &&
               std::find(projects.begin(), projects.end(), path) == projects.end()) {
                projects.push_back(path);
            }
        };

        appendProject(preferredProject);

        QFile recentProjectsFile(recentProjectsPath);
        if(recentProjectsFile.exists()) {
            if(!recentProjectsFile.open(QIODevice::ReadOnly)) {
                aWarning() << "Unable to read recent projects from" << recentProjectsPath.data();
                return;
            }
            const Variant recentProjects = Json::load(TString(recentProjectsFile.readAll().toStdString()));
            recentProjectsFile.close();
            if(!recentProjects.isValid() || recentProjects.type() != MetaType::VARIANTLIST) {
                aWarning() << "Unable to parse recent projects from" << recentProjectsPath.data();
                return;
            }
            for(const Variant &entry : recentProjects.toList()) {
                appendProject(entry.toString());
            }
        }

        for(const QString &entry : legacyProjects) {
            appendProject(TString(entry.toStdString()));
        }

        VariantList serializedProjects;
        for(const TString &path : projects) {
            serializedProjects.push_back(Variant(path));
        }
        const TString serialized = Json::save(Variant(serializedProjects), 0);
        const QByteArray data(serialized.data(), serialized.length());

        QSaveFile output(recentProjectsPath);
        if(!output.open(QIODevice::WriteOnly)) {
            aWarning() << "Unable to write recent projects to" << recentProjectsPath.data();
            return;
        }
        if(output.write(data) != data.size()) {
            aWarning() << "Unable to write the complete recent projects list to" << recentProjectsPath.data();
            return;
        }
        if(!output.commit()) {
            aWarning() << "Unable to save recent projects to" << recentProjectsPath.data();
        }
    }
}

StringList ProjectModel::s_list;

ProjectModel::ProjectModel() :
        QAbstractListModel() {

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant value = settings.value(gProjects);
    if(value.isValid()) {
        for(auto &it : value.toStringList()) {
            TString path(it.toStdString());
            if(File::exists(path)) {
                s_list.push_back(path);

                TString icon(Url(path).absoluteDir() + "/cache/thumbnails/auto.png");

                QImage image(":/Images/icons/thunderlight.svg");
                if(File::exists(icon)) {
                    image = QImage(icon.data());
                }
                image = image.scaledToWidth(64);

                m_iconCache[path] = image;
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
        case Qt::DecorationRole: { return m_iconCache[path]; }
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
    settings.sync();
    if(settings.status() != QSettings::NoError) {
        aWarning() << "Unable to save the World Editor project history to settings.";
    }
    syncRecentProjectsFile(path);
}

void ProjectModel::syncRecentProjects() {
    syncRecentProjectsFile(TString());

}
