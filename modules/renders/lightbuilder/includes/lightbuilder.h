#ifndef LIGHTBUILDER_H
#define LIGHTBUILDER_H

#include <QSettings>
#include <QMainWindow>
#include <QProgressBar>
#include <QListWidgetItem>

#include <vector>
#include <list>

#include <log.h>
#include <file.h>

#include "renderprogress.h"

#include "tracer/scheduler.h"

class QLog;

namespace Ui {
    class LightBuilder;
}

class LightBuilder : public QMainWindow {
    Q_OBJECT
    
public:
    LightBuilder            ();
    ~LightBuilder           ();

    void                    readSettings                    ();
    void                    writeSettings                   (QSettings &settings);

private slots:
    void                    onLogRecord                     (const QString &str);

    void                    on_actionRender_triggered       ();

    void                    on_renderButton_clicked         ();

    void                    on_firstBounceCombo_currentIndexChanged (int index);

    void                    on_secondBounceCombo_currentIndexChanged(int index);

    void                    on_taskEnable_toggled           (bool checked);

    void                    on_taskRect_toggled             (bool checked);

    void                    onAllDone                       ();

    void                    onAllStop                       ();

    void                    updateScene                     (int x, int y);

    void                    on_cellWidthSpin_valueChanged   (int arg1);

    void                    on_photonCheckBox_toggled       (bool checked);

    void                    on_saveCheck_toggled            (bool checked);

    void                    on_renderSettings_visibilityChanged     (bool visible);
    void                    on_globalIllumination_visibilityChanged (bool visible);
    void                    on_cameraSettings_visibilityChanged     (bool visible);
    void                    on_workerSettings_visibilityChanged     (bool visible);

    void                    on_actionRender_Settings_triggered      (bool checked);
    void                    on_actionGlobal_Illumination_triggered  (bool checked);
    void                    on_actionCamera_Settings_triggered      (bool checked);
    void                    on_actionWorker_Settings_triggered      (bool checked);
    void                    on_cameraTargetCheck_clicked            (bool checked);

    void                    on_pathBtn_clicked              ();

    void                    on_devicesList_itemChanged      (QListWidgetItem *item);

private:
    void                    switchBounceFace                (int first, int second);

    void                    updateTitle                     (const QString &path);

    RenderProgress         *pProgress;

    Engine                 *m_pEngine;

    Object                 *pMap;

    Scheduler              *pScheduler;

    Ui::LightBuilder       *ui;

};

#endif // LIGHTBUILDER_H
