#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include "baseobjectmodel.h"

#include <QImage>
#include <QSortFilterProxyModel>

class QFileSystemWatcher;

class ContentTreeFilter : public QSortFilterProxyModel {
public:
    explicit ContentTreeFilter(QObject *parent);

    void setContentTypes(const QStringList &list);

protected:
    bool canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const;

    QStringList m_List;
};

class ContentTree : public BaseObjectModel {
    Q_OBJECT

public:
    static ContentTree *instance();

    static void destroy();

    bool isDir(const QModelIndex &index) const;

    QString path(const QModelIndex &index) const override;

    bool reimportResource(const QModelIndex &index);

    bool removeResource(const QModelIndex &index) override;

    QModelIndex getContent() const;

public slots:
    void onRendered(const QString &uuid);

    void update(const QString &path);

    void clean(QObject *parent);

private:
    ContentTree();
    ~ContentTree() {}

    static ContentTree *m_pInstance;

protected:
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    int columnCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    QObject *m_content;

    QImage m_folder;
};

#endif // CONTENTTREE_H
