#ifndef WORLDBUILDER_H
#define WORLDBUILDER_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QProcess>

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

namespace Ui {
    class SceneComposer;
}

class SceneComposer : public QMainWindow {
    Q_OBJECT

public:
    explicit SceneComposer  (Engine *engine, QWidget *parent = 0);
    ~SceneComposer          ();

    bool                    isModified                                  () const { return mModified; }

    void                    resetModified                               () { mModified = false; }

public slots:
    void                    onObjectSelected                            (Object::ObjectList objects);

private:
    void                    updateTitle                                 ();

    void                    closeEvent                                  (QCloseEvent *event);
    void                    timerEvent                                  (QTimerEvent *);

    bool                    checkSave                                   ();

    Ui::SceneComposer      *ui;

    Engine                 *m_pEngine;

    QMenu                  *cmToolbars;

    PluginDialog           *m_pPluginDlg;

    QObject                *m_pProperties;

    Object                 *m_pMap;

    QString                 mPath;

    ImportQueue            *m_pImportQueue;

    QProcess               *m_pBuilder;

    ByteArray               m_Back;

    bool                    mModified;

private slots:
    void                    onGLInit                                    ();

    void                    onModified                                  ();

    void                    onUndoRedoUpdated                           ();

    void                    readOutput                                  ();

    void                    readError                                   ();

    void                    onFinished                                  (int exitCode, QProcess::ExitStatus);

    void                    parseLogs                                   (const QString &log);

    void                    on_action_New_triggered                     ();
    void                    on_action_Open_triggered                    (const QString &arg = QString());
    void                    on_actionSave_triggered                     ();
    void                    on_actionSave_As_triggered                  ();

    void                    on_actionPlugin_Manager_triggered           ();

    void                    on_actionEditor_Mode_triggered              ();
    void                    on_actionGame_Mode_triggered                ();

    void                    on_actionTake_Screenshot_triggered          ();

    void                    on_actionUndo_triggered                     ();
    void                    on_actionRedo_triggered                     ();

    void                    onToolWindowActionToggled                   (bool checked);

    void                    onToolWindowVisibilityChanged               (QWidget *toolWindow, bool visible);

    void                    on_actionSave_Layout_triggered              ();
    void                    on_actionResore_Layout_triggered            ();
    void                    on_actionBuild_Project_triggered            ();

    void                    on_actionOptions_triggered                  ();
    void                    on_actionAbout_triggered                    ();
};

#endif // WORLDBUILDER_H
