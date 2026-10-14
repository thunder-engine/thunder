#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <window.h>
#include <components/menubar.h>
#include <filedroptarget.h>

#include <utility>
#include <vector>

class Scene;
class Canvas;
class ImportQueue;
class ToolWindowManager;
class SceneComposer;
class HierarchyBrowser;
class Widget;
class AssetEditor;

class MainWindow : public Window {
    A_OBJECT(MainWindow, Window, Editor)

    A_METHODS(
        A_SLOT(MainWindow::onImportFinished),
        A_SLOT(MainWindow::onToolWindowVisibilityChanged),
        A_SLOT(MainWindow::onCurrentToolWindowChanged),
        A_SLOT(MainWindow::onContentBrowserOpen)
    )

public:
    MainWindow();
    ~MainWindow() override;

    bool create();

    void onOpenProject(const TString &path);

    void onImportFinished();

private:
    void initUI();
    void createMenus(SceneComposer *composer);
    void onWindowMenuAction(Widget *toolWindow);
    void onToolWindowVisibilityChanged(Widget *toolWindow);
    void onCurrentToolWindowChanged(Widget *toolWindow);
    void onContentBrowserOpen(const TString &path);

    void beforeRender() override;

private:
    Scene *m_scene;
    Canvas *m_canvas;
    ToolWindowManager *m_toolWindowManager;
    MenuBar *m_menuBarWidget;
    MenuBar::MenuModel *m_windowMenu;
    std::vector<std::pair<Widget *, size_t>> m_windowMenuItems;
    HierarchyBrowser *m_hierarchyBrowser;
    AssetEditor *m_currentAssetEditor;
    FileDropTarget m_fileDropTarget;

    ImportQueue *m_queue;

};

#endif // MAINWINDOW_H
