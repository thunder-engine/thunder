#ifndef WORLDBUILDER_H
#define WORLDBUILDER_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QProcess>
#include <QMenu>

#include <vector>
#include <cstdint>

#include <amath.h>
#include <engine.h>

using namespace std;

class QLog;

class Object;
class PluginDialog;
class ProjectManager;
class SceneView;
class ImportQueue;

class ProjectModel;
class FeedManager;

class IConverterSettings;

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public QMainWindow {
    Q_OBJECT

public:
    explicit SceneComposer  (Engine *engine, QWidget *parent = nullptr);
    ~SceneComposer          ();

public slots:
    void                    onObjectSelected                            (Object::ObjectList objects);
    void                    onAssetSelected                             (IConverterSettings *settings);

    void                    onOpenProject                               (const QString &path);

private:
    void                    updateTitle                                 ();

    void                    closeEvent                                  (QCloseEvent *event);
    void                    timerEvent                                  (QTimerEvent *);
    void                    resizeEvent                                 (QResizeEvent *);

    bool                    checkSave                                   ();

    void                    saveWorkspace                               ();
    void                    resetWorkspace                              ();

    void                    findWorkspaces                              (const QString &dir);

    void                    checkImportSettings                         (IConverterSettings *settings);

    Ui::SceneComposer      *ui;

    Engine                 *m_pEngine;

    QMenu                  *cmToolbars;

    QObject                *m_pProperties;

    QString                 m_Path;

    QString                 m_CurrentWorkspace;

    QProcess               *m_pBuilder;

    ImportQueue            *m_pQueue;

    ByteArray               m_Back;

    ProjectModel           *m_pProjectModel;
    FeedManager            *m_pFeedManager;

    QAction                *m_Undo;
    QAction                *m_Redo;

private slots:
    void                    onSettingsUpdated                           ();

    void                    onNewProject                                ();
    void                    onImportProject                             ();

    void                    onBuildProject                              ();

    void                    onOpen                                      (const QString &arg = QString());

    void                    onImportFinished                            ();

    void                    readOutput                                  ();

    void                    readError                                   ();

    void                    onFinished                                  (int exitCode, QProcess::ExitStatus);

    void                    parseLogs                                   (const QString &log);

    void                    on_commitButton_clicked                     ();
    void                    on_revertButton_clicked                     ();

    void                    on_actionNew_triggered                      ();
    void                    on_actionSave_triggered                     ();
    void                    on_actionSave_As_triggered                  ();

    void                    on_actionPlugin_Manager_triggered           ();

    void                    on_actionEditor_Mode_triggered              ();
    void                    on_actionGame_Mode_triggered                ();

    void                    on_actionTake_Screenshot_triggered          ();

    void                    on_actionUndo_triggered                     ();
    void                    on_actionRedo_triggered                     ();

    void                    onWorkspaceActionClicked                    ();
    void                    onToolWindowActionToggled                   (bool state);

    void                    onToolWindowVisibilityChanged               (QWidget *toolWindow, bool visible);

    void                    on_actionSave_Workspace_triggered           ();
    void                    on_actionReset_Workspace_triggered          ();

    void                    on_actionOptions_triggered                  ();
    void                    on_actionAbout_triggered                    ();
    void                    on_actionNew_Object_triggered               ();
};

#endif // WORLDBUILDER_H
