#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractListModel>

class ProjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        DirRole
    };

    ProjectModel            ();

    ~ProjectModel           ();

    int                     rowCount    (const QModelIndex &parent) const;

    QVariant                data        (const QModelIndex &index, int role) const;

    QHash<int, QByteArray>  roleNames   () const;

    void                    addProject  (const QString &path);

protected:
    QStringList             m_List;

};

#endif // PROJECTMODEL_H
