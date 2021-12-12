#ifndef MAINWIDOW_H
#define MAINWIDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QMenu>

#include <vector>
#include <cstdint>

#include <amath.h>
#include <engine.h>

#include "aboutdialog.h"
#include "managers/pluginmanager/plugindialog.h"

using namespace std;

class Object;
class ImportQueue;

class ProjectModel;
class FeedManager;
class DocumentModel;

class AssetConverterSettings;
class AssetEditor;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Engine *engine, QWidget *parent = nullptr);
    ~MainWindow() Q_DECL_OVERRIDE;

public slots:
    void onItemSelected(QObject *item);
    void onOpenEditor(const QString &path);

    void onOpenProject(const QString &path);

private:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

    void saveWorkspace();
    void resetWorkspace();
    void resetGeometry();

    void findWorkspaces(const QString &dir);

    void setGameMode(bool game);

private:
    Ui::MainWindow *ui;

    Engine *m_Engine;

    QString m_CurrentWorkspace;

    ImportQueue *m_Queue;

    ProjectModel *m_ProjectModel;
    FeedManager *m_FeedManager;
    DocumentModel *m_DocumentModel;

    QAction *m_Undo;
    QAction *m_Redo;

    AssetEditor *m_MainDocument;
    AssetEditor *m_CurrentDocument;

    AboutDialog m_aboutDlg;
    PluginDialog m_pluginDlg;

private slots:
    void onSettingsUpdated();

    void onNewProject();
    void onImportProject();

    void onBuildProject();

    void onImportFinished();

    void on_commitButton_clicked();
    void on_revertButton_clicked();

    void on_actionNew_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();

    void on_actionEditor_Mode_triggered();
    void on_actionGame_Mode_triggered();

    void on_actionTake_Screenshot_triggered();

    void onWorkspaceActionClicked();
    void onToolWindowActionToggled(bool state);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);
    void onCurrentToolWindowChanged(QWidget *toolWindow);

    void on_actionSave_Workspace_triggered();
    void on_actionReset_Workspace_triggered();

    void on_menuFile_aboutToShow();
    void on_actionReport_Issue_triggered();
    void on_actionAPI_Reference_triggered();
    void on_actionThunder_Answers_triggered();
    void on_actionThunder_Manual_triggered();
    void on_actionExit_triggered();
};

#endif // MAINWIDOW_H
