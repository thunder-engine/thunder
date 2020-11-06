#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QWidget>
#include <QFileInfo>
#include <QListView>
#include <QMenu>
#include <QDropEvent>

#include <engine.h>

#include <editor/converter.h>

class QSortFilterProxyModel;
class AssetItemDeligate;
class AssetFilter;

namespace Ui {
    class AssetBrowser;
}

class AssetBrowser : public QWidget {
    Q_OBJECT

public:
    enum FileTypes {
        Dir,
        File
    };

public:
    AssetBrowser(QWidget *parent);
    ~AssetBrowser();

    void filterByType(const QString &);

    void setSelected(const QString &resource);

    QImage icon(const QString &resource) const;

signals:
    void assetSelected(const QString &uuid);

protected:
    AssetItemDeligate *m_pContentDeligate;

    AssetFilter *m_pContentProxy;

private slots:
    void on_assetList_clicked(const QModelIndex &index);

    void on_findContent_textChanged(const QString &arg1);

    void onModelUpdated();

private:
    Ui::AssetBrowser *ui;

    QString m_Resource;

};

#endif // CONTENTBROWSER_H
