#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

#include <QImage>

class TString;

class ContentTree : public BaseObjectModel {
    Q_OBJECT

public:
    static ContentTree *instance();

    QString path(const QModelIndex &index) const override;

    bool reimportResource(const QModelIndex &index);

    bool removeResource(const QModelIndex &index) override;

    QModelIndex getContent() const;

    QModelIndex setNewAsset(const QString &name, const QString &source = QString(), bool directory = false);

public slots:
    void onRendered(const TString &uuid);

    void update();

    void clean(QObject *parent);

    void revert() override;

private:
    ContentTree();
    ~ContentTree() {}

protected:
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    int columnCount(const QModelIndex &) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    QObject *m_content;

    QObject *m_newAsset;

    QImage m_folder;
};

#endif // CONTENTTREE_H
