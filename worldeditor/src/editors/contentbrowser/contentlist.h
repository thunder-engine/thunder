#ifndef CONTENTLIST_H
#define CONTENTLIST_H

#include <stdint.h>

#include <QFileInfo>
#include <QImage>

#include <QMimeData>
#include <QDir>

#include <patterns/asingleton.h>

#include "baseobjectmodel.h"

class QOpenGLContext;

class ProjectManager;
class AssetManager;

class Engine;

class ContentList  : public BaseObjectModel, public ASingleton<ContentList> {
    Q_OBJECT

public:
    void                        init                        (Engine *engine);

    int                         columnCount                 (const QModelIndex &parent) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

    bool                        setData                     (const QModelIndex &index, const QVariant &value, int role);

    bool                        isDir                       (const QModelIndex &index) const;

    bool                        reimportResource            (const QModelIndex &index);
    bool                        removeResource              (const QModelIndex &index);

    QImage                      icon                        (const QModelIndex &index) const;

    QString                     path                        (const QModelIndex &index) const;

    QModelIndex                 findResource                (const QString &resource) const;

    void setRootPath(const QString &path) {
        m_rootPath  = path;
    }

    QString rootPath() const {
        return m_rootPath;
    }

public slots:
    void                        onRendered                  (const QString &uuid);

protected slots:
    void                        update                      (const QString &path);

protected:
    Qt::DropActions             supportedDropActions        () const;
    QStringList                 mimeTypes                   () const;
    QMimeData                  *mimeData                    (const QModelIndexList &indexes) const;
    bool                        canDropMimeData             (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const;
    bool                        dropMimeData                (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent);

protected:
    friend class ASingleton<ContentList>;

    ContentList                 ();
    ~ContentList                () {}

    Engine                     *m_pEngine;

    QImage                      m_DefaultIcon;


    ProjectManager             *m_pProjectManager;
    AssetManager               *m_pAssetManager;

    QString                     m_rootPath;
};

#endif // CONTENTLIST_H
