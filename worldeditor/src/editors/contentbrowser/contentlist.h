#ifndef CONTENTLIST_H
#define CONTENTLIST_H

#include <stdint.h>

#include <QFileInfo>
#include <QImage>

#include <QMimeData>
#include <QDir>
#include <QSortFilterProxyModel>

#include "baseobjectmodel.h"

class ProjectManager;
class AssetManager;

class Engine;

class ContentListFilter : public QSortFilterProxyModel {
public:
    typedef QList<int32_t> TypeList;

    explicit ContentListFilter(QObject *parent);

    void setContentTypes(const TypeList &list);

    QString rootPath() const;

    void setRootPath(const QString &path);

protected:
    bool canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkRootPath(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const;

    TypeList m_List;

    QString m_rootPath;
};

class ContentList  : public BaseObjectModel {
    Q_OBJECT

public:
    static ContentList         *instance                    ();

    static void                 destroy                     ();

    void                        init                        (Engine *engine);

    int                         columnCount                 (const QModelIndex &parent) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

    bool                        setData                     (const QModelIndex &index, const QVariant &value, int role);

    bool                        isDir                       (const QModelIndex &index) const;

    bool                        reimportResource            (const QModelIndex &index);
    bool                        removeResource              (const QModelIndex &index);

    QString                     path                        (const QModelIndex &index) const;

    QModelIndex                 findResource                (const QString &resource) const;

public slots:
    void                        onRendered                  (const QString &uuid);

    void                        update                      ();

protected:
    Qt::DropActions             supportedDropActions        () const;
    QStringList                 mimeTypes                   () const;
    QMimeData                  *mimeData                    (const QModelIndexList &indexes) const;

    void                        scan                        (const QString &path);

private:
    ContentList                 ();
    ~ContentList                () {}

    static ContentList         *m_pInstance;

protected:
    Engine                     *m_pEngine;

    QImage                      m_DefaultIcon;

    ProjectManager             *m_pProjectManager;
    AssetManager               *m_pAssetManager;

    QObject                    *m_pContent;
};

#endif // CONTENTLIST_H
