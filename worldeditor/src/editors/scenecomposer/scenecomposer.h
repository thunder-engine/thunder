#ifndef WORLDBUILDER_H
#define WORLDBUILDER_H

#include <QMainWindow>
#include <QProcess>
#include <QMenu>

#include <vector>
#include <cstdint>

#include <amath.h>
#include <engine.h>

using namespace std;

class Object;
class ImportQueue;

class ProjectModel;
class FeedManager;
class DocumentModel;

class IConverterSettings;

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public QMainWindow {
    Q_OBJECT

public:
    explicit SceneComposer(Engine *engine, QWidget *parent = nullptr);
    ~SceneComposer() Q_DECL_OVERRIDE;

public slots:
    void onObjectSelected(Object::ObjectList objects);
    void onAssetSelected(IConverterSettings *settings);
    void onItemSelected(QObject *item);
    void onOpenEditor(const QString &path);

    void onOpenProject(const QString &path);

private:
    void updateTitle();

    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

    bool checkSave();

    void saveWorkspace();
    void resetWorkspace();
    void resetGeometry();

    void findWorkspaces(const QString &dir);

    void checkImportSettings(IConverterSettings *settings);

    Ui::SceneComposer *ui;

    Engine *m_Engine;

    QObject *m_Properties;

    QString m_Path;

    QString m_CurrentWorkspace;

    QProcess *m_Builder;

    ImportQueue *m_Queue;

    ByteArray m_Back;

    ProjectModel *m_ProjectModel;
    FeedManager *m_FeedManager;
    DocumentModel *m_DocumentModel;

    QAction *m_Undo;
    QAction *m_Redo;

    QWidget *m_MainDocument;
    QWidget *m_CurrentDocument;

private slots:
    void onSettingsUpdated();

    void onRepickSelected();

    void onNewProject();
    void onImportProject();

    void onBuildProject();

    void onOpen(const QString &arg = QString());

    void onImportFinished();

    void readOutput();

    void readError();

    void onFinished(int exitCode, QProcess::ExitStatus);

    void parseLogs(const QString &log);

    void on_commitButton_clicked();
    void on_revertButton_clicked();

    void on_actionNew_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();

    void on_actionPlugin_Manager_triggered();

    void on_actionEditor_Mode_triggered();
    void on_actionGame_Mode_triggered();

    void on_actionTake_Screenshot_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();

    void onWorkspaceActionClicked();
    void onToolWindowActionToggled(bool state);

    void onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible);
    void onCurrentToolWindowChanged(QWidget *toolWindow);

    void on_actionSave_Workspace_triggered();
    void on_actionReset_Workspace_triggered();

    void on_actionOptions_triggered();
    void on_actionAbout_triggered();
    void on_actionNew_Object_triggered();

    void on_menuFile_aboutToShow();
};

#endif // WORLDBUILDER_H
