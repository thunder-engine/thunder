#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractListModel>
#include <QImage>

class ProjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    ProjectModel();

    ~ProjectModel();

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    static void addProject(const QString &path);

protected:
    QStringList m_list;

    mutable QMap<QString, QImage> m_iconCache;
};

#endif // PROJECTMODEL_H
