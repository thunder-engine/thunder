#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QWidget>
#include <QFileInfo>
#include <QListView>
#include <QMenu>
#include <QDropEvent>

#include <engine.h>

#include "converters/converter.h"

class ContentItemDeligate;
class ContentListFilter;
class ContentTreeFilter;
class BaseObjectModel;

namespace Ui {
    class ContentBrowser;
}

class ContentBrowser : public QWidget {
    Q_OBJECT

public:
    enum ContentTypes {
        Text                  = IConverter::ContentText,
        Texture               = IConverter::ContentTexture,
        Material              = IConverter::ContentMaterial,
        Mesh                  = IConverter::ContentMesh,
        Atlas                 = IConverter::ContentAtlas,
        Font                  = IConverter::ContentFont,
        Animation             = IConverter::ContentAnimation,
        Effect                = IConverter::ContentEffect,
        Sound                 = IConverter::ContentSound,
        Code                  = IConverter::ContentCode,
        Map                   = IConverter::ContentMap,
        Pipeline              = IConverter::ContentPipeline,
        Prefab                = IConverter::ContentPrefab,
        AnimationStateMachine = IConverter::ContentAnimationStateMachine,
        Pose                  = IConverter::ContentPose
    };

    enum FileTypes {
        Dir,
        File
    };

    Q_ENUM(ContentTypes)
public:
    ContentBrowser          (QWidget *parent);

    ~ContentBrowser         ();

    void                    rescan                  ();

protected:
    void                    readSettings            ();
    void                    writeSettings           ();
    void                    createContextMenus      ();

    ContentItemDeligate    *m_pContentDeligate;

    ContentListFilter      *m_pContentProxy;

    ContentTreeFilter      *m_pTreeProxy;

    QMenu                  *m_pFilterMenu;

private slots:
    void                    onCreationMenuTriggered         (QAction *);

    void                    onFilterMenuTriggered           (QAction *);

    void                    onItemDuplicate                 ();
    void                    onItemRename                    ();
    void                    onItemDelete                    ();
    void                    onItemReimport                  ();

    void                    on_contentTree_clicked          (const QModelIndex &index);
    void                    on_contentList_doubleClicked    (const QModelIndex &index);

    void                    on_contentTree_customContextMenuRequested   (const QPoint &pos);
    void                    on_contentList_customContextMenuRequested   (const QPoint &pos);
    void                    on_findContent_textChanged                  (const QString &arg1);

    void                    showInGraphicalShell            ();

private:
    QAction*                createAction                    (const QString &name, const char *member, const QKeySequence &shortcut = 0);

private:
    Ui::ContentBrowser     *ui;

    QMenu                   m_ContentMenu;

    QMenu                   m_CreationMenu;

    QMenu                   m_contentTreeMenu;

};

#endif // CONTENTBROWSER_H
