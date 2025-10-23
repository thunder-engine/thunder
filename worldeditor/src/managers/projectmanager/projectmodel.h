#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractListModel>
#include <QImage>

#include <astring.h>

class ProjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    ProjectModel();

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    static void addProject(const QString &path);

protected:
    static StringList s_list;

    mutable std::map<QString, QImage> m_iconCache;
};

#endif // PROJECTMODEL_H
