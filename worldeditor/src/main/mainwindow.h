#ifndef MAINWIDOW_H
#define MAINWIDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QMenu>

#include <amath.h>
#include <engine.h>

#include "aboutdialog.h"
#include "managers/plugindialog/plugindialog.h"

class ImportQueue;

class ProjectModel;
class FeedManager;
class DocumentModel;

class Preview;
class AssetEditor;
class EditorGadget;

class EditorSettingsBrowser;
class ProjectSettingsBrowser;
class ContentBrowser;
class ConsoleManager;

class ProjectSettings;
class EditorSettings;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(Engine *engine, QWidget *parent = nullptr);
    ~MainWindow() Q_DECL_OVERRIDE;

public slots:
    void onOpenEditor(const QString &path);

    void onOpenProject(const QString &path);

signals:
    void readBuildLogs(QString log);

private:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

    void addGadget(EditorGadget *gadget);

    AssetEditor *openEditor(const QString &path);

    void resetGeometry();

    void findWorkspaces(const QString &dir);

    void setGameMode(bool game);

    void build(QString platform);

private:
    Ui::MainWindow *ui;

    AboutDialog m_aboutDlg;
    PluginDialog m_pluginDlg;

    QString m_currentWorkspace;

    QList<EditorGadget *> m_gadgets;

    Engine *m_engine;

    ImportQueue *m_queue;

    ProjectModel *m_projectModel;
    FeedManager *m_feedManager;
    DocumentModel *m_documentModel;

    EditorSettings *m_editorSettings;
    ProjectSettings *m_projectSettings;

    EditorSettingsBrowser *m_editorSettingsBrowser;
    ProjectSettingsBrowser *m_projectSettingsBrowser;
    ContentBrowser *m_contentBrowser;
    ConsoleManager *m_consoleOutput;

    Preview *m_preview;

    QAction *m_undo;
    QAction *m_redo;

    AssetEditor *m_mainEditor;
    AssetEditor *m_currentEditor;

    QProcess *m_builder;

    bool m_forceReimport;

private slots:
    void onNewProject();
    void onImportProject();

    void onBuildProject();

    void onImportFinished();

    void onBuildFinished(int exitCode, QProcess::ExitStatus);
    void readOutput();
    void readError();

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();

    void on_actionPlay_triggered();
    void on_actionPause_triggered();

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
