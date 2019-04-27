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
class ContentFilter;

namespace Ui {
    class ContentBrowser;
}

class ContentBrowser : public QWidget {
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
    ContentBrowser          (QWidget *parent);

    ~ContentBrowser         ();

    void                    setCompact              (bool value);

    void                    filterByType            (const uint32_t);

    void                    setSelected             (const QString &resource);

    QImage                  icon                    (const QString &resource) const;

signals:
    void                    assetSelected           (IConverterSettings *settings);

protected:
    void                    readSettings            ();
    void                    writeSettings           ();

    AssetItemDeligate      *m_pContentDeligate;

    ContentFilter          *m_pContentProxy;

    QSortFilterProxyModel  *m_pTreeProxy;

    QMenu                  *m_pFilterMenu;

private slots:
    void                    onCreationMenuTriggered         (QAction *);

    void                    onFilterMenuTriggered           (QAction *);

    void                    onItemDuplicate                 ();
    void                    onItemRename                    ();
    void                    onItemDelete                    ();
    void                    onItemReimport                  ();

    void                    onContentUpdated                ();

    void                    on_contentTree_clicked          (const QModelIndex &index);
    void                    on_contentList_clicked          (const QModelIndex &index);
    void                    on_contentList_doubleClicked    (const QModelIndex &index);

    void                    on_contentList_customContextMenuRequested   (const QPoint &pos);
    void                    on_findContent_textChanged                  (const QString &arg1);

    void                    showInGraphicalShell            ();

private:
    void                    createAction                    (const QString &name, const char *member, const QKeySequence &shortcut = 0);

private:
    Ui::ContentBrowser     *ui;

    bool                    m_Compact;

    QMenu                   m_ContentMenu;

    QMenu                   m_CreationMenu;

    IConverterSettings     *m_pSelected;

};

#endif // CONTENTBROWSER_H
