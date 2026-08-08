#ifndef MAINWIDOW_H
#define MAINWIDOW_H

#include <QMainWindow>
#include <QProcess>

#include <amath.h>
#include <engine.h>

#include "aboutdialog.h"
#include "managers/plugindialog/plugindialog.h"

class ImportQueue;

class Preview;
class AssetEditor;
class EditorGadget;

class EditorSettingsBrowser;
class ProjectSettingsBrowser;
class ContentBrowser;
class ConsoleManager;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onOpenEditor(const TString &path);

    void onOpenProject(const TString &path, Engine &engine);

signals:
    void readBuildLogs(QString log);

private:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

    void timerEvent(QTimerEvent *) override;

    AssetEditor *openEditor(const TString &path);

    void resetGeometry();

    void findWorkspaces(const QString &dir);

    void setGameMode(bool game);

    void build(QString platform);

private:
    Ui::MainWindow *ui;

    AboutDialog m_aboutDlg;
    PluginDialog m_pluginDlg;

    QString m_currentWorkspace;

    ImportQueue *m_queue;

    EditorSettingsBrowser *m_editorSettingsBrowser;
    ProjectSettingsBrowser *m_projectSettingsBrowser;
    ContentBrowser *m_contentBrowser;
    ConsoleManager *m_consoleOutput;

    Preview *m_preview;

    QProcess *m_builder;

private slots:
    void onBuildProject();

    void onImportFinished();

    void onCopyPasteChanged();

    void onBuildFinished(int exitCode, QProcess::ExitStatus);
    void readOutput();
    void readError();

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionPlay_triggered();
    void on_actionPause_triggered();

    void onWorkspaceActionClicked();
    void onToolWindowActionToggled(bool state);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);
    void onCurrentToolWindowChanged(QWidget *toolWindow);

    void on_actionSave_Workspace_triggered();
    void on_actionReset_Workspace_triggered();

    void on_menuFile_aboutToShow();
    void on_menuEdit_aboutToShow();
    void on_actionReport_Issue_triggered();
    void on_actionAPI_Reference_triggered();
    void on_actionThunder_Answers_triggered();
    void on_actionThunder_Manual_triggered();
    void on_actionExit_triggered();

};

#endif // MAINWIDOW_H
