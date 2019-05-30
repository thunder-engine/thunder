#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QWidget>
#include <QFileInfo>
#include <QListView>
#include <QMenu>
#include <QDropEvent>

#include <engine.h>

#include "converters/converter.h"

class QSortFilterProxyModel;
class AssetItemDeligate;
class AssetFilter;

namespace Ui {
    class AssetBrowser;
}

class AssetBrowser : public QWidget {
    Q_OBJECT

public:
    enum ContentTypes {
        Text        = IConverter::ContentText,
        Texture     = IConverter::ContentTexture,
        Material    = IConverter::ContentMaterial,
        Static      = IConverter::ContentMesh,
        Animation   = IConverter::ContentAnimation,
        Effect      = IConverter::ContentEffect,
        Sound       = IConverter::ContentSound,
        Code        = IConverter::ContentCode,
        Map         = IConverter::ContentMap
    };

    enum FileTypes {
        Dir,
        File
    };

    Q_ENUM(ContentTypes)
public:
    AssetBrowser            (QWidget *parent);

    ~AssetBrowser           ();

    void                    filterByType            (const int32_t);

    void                    setSelected             (const QString &resource);

    QImage                  icon                    (const QString &resource) const;

signals:
    void                    assetSelected           (IConverterSettings *settings);

protected:
    AssetItemDeligate      *m_pContentDeligate;

    AssetFilter            *m_pContentProxy;

private slots:
    void                    on_assetList_clicked        (const QModelIndex &index);

    void                    on_findContent_textChanged  (const QString &arg1);

private:
    Ui::AssetBrowser       *ui;

    IConverterSettings     *m_pSelected;

};

#endif // CONTENTBROWSER_H
