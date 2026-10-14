#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <window.h>

class Scene;
class Canvas;
class ImportQueue;
class ToolWindowManager;

class MainWindow : public Window {
    A_OBJECT(MainWindow, Window, Editor)

    A_METHODS(
        A_SLOT(MainWindow::onImportFinished)
    )

public:
    MainWindow();

    bool create();

    void onOpenProject(const TString &path);

    void onImportFinished();

private:
    void initUI();

    void beforeRender() override;

private:
    Scene *m_scene;
    Actor *m_canvas;
    ToolWindowManager *m_toolWindowManager;

    ImportQueue *m_queue;

};

#endif // MAINWINDOW_H
