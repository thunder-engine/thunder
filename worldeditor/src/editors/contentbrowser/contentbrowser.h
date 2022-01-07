#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QWidget>
#include <QFileInfo>
#include <QListView>
#include <QMenu>
#include <QDropEvent>

#include <engine.h>

#include <editor/assetconverter.h>

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
    enum FileTypes {
        Dir,
        File
    };

public:
    ContentBrowser(QWidget *parent);
    ~ContentBrowser();

    void rescan();

public slots:
    void assetUpdated();

signals:
    void assetSelected(AssetConverterSettings *settings);
    void openEditor(const QString &path);

protected:
    void readSettings();
    void writeSettings();
    void createContextMenus();

    ContentItemDeligate *m_pContentDeligate;
    ContentTreeFilter *m_pListProxy;
    ContentTreeFilter *m_pTreeProxy;

    QMenu *m_pFilterMenu;

private slots:
    void onCreationMenuTriggered(QAction *);
    void onFilterMenuTriggered(QAction *);
    void onFilterMenuAboutToShow();

    void onItemOpen();
    void onItemDuplicate();
    void onItemRename();
    void onItemDelete();
    void onItemReimport();

    void on_contentTree_clicked(const QModelIndex &index);
    void on_contentList_doubleClicked(const QModelIndex &index);

    void on_contentTree_customContextMenuRequested(const QPoint &pos);
    void on_contentList_customContextMenuRequested(const QPoint &pos);
    void on_findContent_textChanged(const QString &arg1);

    void on_contentList_clicked(const QModelIndex &index);

    void showInGraphicalShell();

private:
    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

    void changeEvent(QEvent *event) override;

private:
    Ui::ContentBrowser  *ui;

    QMenu m_ContentMenu;
    QMenu m_CreationMenu;
    QMenu m_contentTreeMenu;

};

#endif // CONTENTBROWSER_H
