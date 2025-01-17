#ifndef CONTENTBROWSER_H
#define CONTENTBROWSER_H

#include <QWidget>
#include <QMenu>

#include <engine.h>

#include <editor/assetconverter.h>

class ContentItemDeligate;
class ContentTreeFilter;

class CommitRevert;

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

    void createContextMenus();

    QWidget *commitRevert();

signals:
    void assetsSelected(const QList<QObject *> &settings);
    void openEditor(const QString &path);

protected:
    void readSettings();
    void writeSettings();

    ContentItemDeligate *m_contentDeligate;
    ContentTreeFilter *m_listProxy;
    ContentTreeFilter *m_treeProxy;

    QMenu *m_filterMenu;

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

    void importAsset();

private:
    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

    void changeEvent(QEvent *event) override;

private:
    Ui::ContentBrowser *ui;

    QMenu m_contentMenu;
    QMenu m_creationMenu;
    QMenu m_contentTreeMenu;

    CommitRevert *m_commitRevert;

    AssetConverterSettings *m_settings;

};

#endif // CONTENTBROWSER_H
