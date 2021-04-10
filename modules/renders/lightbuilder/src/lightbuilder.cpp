#include "lightbuilder.h"
#include "ui_lightbuilder.h"

#include <QFileDialog>

#include <memory>

#define TITLE       "LightBuilder"

LightBuilder::LightBuilder() :
        QMainWindow(),
        ui(new Ui::LightBuilder) {

    ui->setupUi(this);

    //pScheduler  = new Scheduler(m_pEngine);

    pMap = nullptr;

    pProgress = new RenderProgress(this);

    ui->firstBounceCombo->addItem(tr("Simple back tracing"), BACK_TRACING);
    ui->firstBounceCombo->addItem(tr("Photon mapping"), PHOTON_MAPPING);
    ui->firstBounceCombo->addItem(tr("Brute force"), PATH_TRACING);

    ui->secondBounceCombo->addItem(tr("Simple back tracing"), BACK_TRACING);
    ui->secondBounceCombo->addItem(tr("Photon mapping"), PHOTON_MAPPING);
    ui->secondBounceCombo->addItem(tr("Brute force"), PATH_TRACING);

    bool hardware = pScheduler->isHardware();
    ui->groupBoxHardware->setEnabled(hardware);
    if(hardware) {
        std::vector<std::string> list;
        pScheduler->oclDevices(list);
        for(uint32_t i = 0; i < list.size(); i++) {
            QListWidgetItem* item = new QListWidgetItem(QString(list[i].c_str()).trimmed(), ui->devicesList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setData(Qt::UserRole, i);

            ui->devicesList->addItem(item);
        }
    }

    ui->spinMaxThreads->setValue(QThread::idealThreadCount());

    connect(pScheduler, SIGNAL(updateResult(QRect)), ui->label, SLOT(onUpdateResult(QRect)));
    connect(pScheduler, SIGNAL(updateProgress(float, int)), pProgress, SLOT(onUpdateProgress(float, int)));
    connect(pScheduler, SIGNAL(allDone()), this, SLOT(onAllDone()));

    connect(pProgress, SIGNAL(stop()), this, SLOT(onAllStop()));
    //connect(log, SIGNAL(postRecord(const QString&)), this, SLOT(onLogRecord(const QString &)));
}

LightBuilder::~LightBuilder() {
    delete ui;

    delete pProgress;

    delete pScheduler;

    delete m_pEngine;
}

void LightBuilder::readSettings() {
/*
    QSettings settings(COMPANY_NAME, PRODUCT_NAME);
    restoreGeometry(settings.value("light.geometry").toByteArray());
    restoreState(settings.value("light.windows").toByteArray());
*/
}

void LightBuilder::writeSettings(QSettings &settings) {
/*
    settings.setValue("light.geometry", saveGeometry());
    settings.setValue("light.windows", saveState());
*/
}

void LightBuilder::switchBounceFace(int first, int second) {
    ui->groupPhoton->hide();
    ui->groupBruteForce->hide();

    switch(first) {
        case BACK_TRACING: {
            //ui->groupSimple->show();
        } break;

        case PHOTON_MAPPING: {
            ui->groupPhoton->show();
        } break;

        case PATH_TRACING: {
            ui->groupBruteForce->show();
            ui->bruteMaxBouncesSpin->setDisabled(true);
        } break;

        default: break;
    }

    switch(second) {
        case BACK_TRACING: {
            //ui->groupSimple->show();
        } break;

        case PHOTON_MAPPING: {
            ui->groupPhoton->show();
        } break;

        case PATH_TRACING: {
            ui->groupBruteForce->show();
            ui->bruteMaxBouncesSpin->setEnabled(true);
        } break;

        default: break;
    }
}

void LightBuilder::updateTitle(const QString &path) {
    if(!path.isEmpty()) {
        setWindowTitle(path + " - " + QString(TITLE));
    } else {
        setWindowTitle(TITLE);
    }
}

void LightBuilder::updateScene(int x, int y) {
/*
    if(ui->actionRT_Mode->isChecked()) {
        pScheduler->restart();
    }
*/
}

void LightBuilder::onLogRecord(const QString &str) {
    ui->plainTextEdit->appendHtml(str);
}


void LightBuilder::on_actionRender_triggered() {
    pProgress->show();
    pProgress->onUpdateProgress(0.0f, 0);

    int w   = ui->widthSpin->value();
    int h   = ui->heightSpin->value();

    if(ui->taskEnable->isChecked()) {
        if(ui->taskRect->isChecked()) {
            pScheduler->setCellSize(ui->cellWidthSpin->value(), ui->cellWidthSpin->value());
        } else {
            pScheduler->setCellSize(ui->cellWidthSpin->value(), ui->cellHeightSpin->value());
        }
    } else {
        pScheduler->setCellSize(w, h);
    }

    pScheduler->setMaxThreads(ui->spinMaxThreads->value());

    if(pScheduler->isHardware()) {
        pScheduler->addDevice(-1);
        for(int i = 0; i < ui->devicesList->count(); i++) {
            if(ui->devicesList->item(i)->checkState() == Qt::Checked) {
                pScheduler->addDevice(ui->devicesList->item(i)->data(Qt::UserRole).toInt());
            }
        }
    }

    pScheduler->setCameraSettings(ui->fNumberSpin->value(),
                                 ui->cameraTargetCheck->isChecked(),
                                 ui->focalLengthSpin->value(),
                                 ui->depthOfFieldCheck->isChecked(),
                                 ui->motionBlourCheck->isChecked(),
                                 ui->cameraSubdivsSpin->value());

    int first   = ui->firstBounceCombo->itemData(ui->firstBounceCombo->currentIndex()).toInt();
    int second  = ui->secondBounceCombo->itemData(ui->secondBounceCombo->currentIndex()).toInt();
    pScheduler->setBounceEngines((CollectTypes)first, (CollectTypes)second);

    pScheduler->setPathTracing(ui->bruteSubdivSpin->value(),
                              ui->bruteMaxBouncesSpin->value());

    pScheduler->setPhotonMapping(ui->photonSamplesSpin->value(),
                                ui->photonDiffuseSubSpin->value(),
                                ui->photonCausticSubSpin->value(),
                                ui->photonCheckBox->isChecked(),
                                ui->photonDistanceSpin->value(),
                                ui->photonMaxPhotonsSpin->value(),
                                ui->photonSeedSpin->value(),
                                ui->photonTreeSpin->value());

    bool bake   = ui->bakingCheckBox->isChecked();
    /// \todo return this
    //pScheduler->setScene(&mScene, bake);
    //if(bake) {
    //    ui->label->setUV(pScheduler->uv());
    //}

    bool reverse    = (ui->taskEnable->isChecked() && ui->taskReverse->isChecked());
    ui->label->setResult(pScheduler->create(w, h, reverse, bake), w, h);

    pScheduler->start();
}

void LightBuilder::on_renderButton_clicked() {
    on_actionRender_triggered();
}

void LightBuilder::onAllDone() {
    pProgress->hide();
    if(ui->saveCheck->isChecked() && !ui->pathEdit->text().isEmpty()) {
        QString stamp = QDate::currentDate().toString("ddMMyy") + "_" + QTime::currentTime().toString("HHmmss") + ".png";
        ui->label->result().save(ui->pathEdit->text() + "/" + stamp);
    }
}

void LightBuilder::onAllStop() {
    pScheduler->stop();
}

void LightBuilder::on_firstBounceCombo_currentIndexChanged(int index) {
    int first   = ui->firstBounceCombo->itemData(index).toInt();
    int second  = ui->secondBounceCombo->itemData(ui->secondBounceCombo->currentIndex()).toInt();

    switchBounceFace(first, second);
}

void LightBuilder::on_secondBounceCombo_currentIndexChanged(int index) {
    int first   = ui->firstBounceCombo->itemData(ui->firstBounceCombo->currentIndex()).toInt();
    int second   = ui->secondBounceCombo->itemData(index).toInt();

    switchBounceFace(first, second);
}

void LightBuilder::on_taskEnable_toggled(bool checked) {
    ui->taskRect->setEnabled(checked);
    ui->labelKeepRect->setEnabled(checked);

    ui->taskReverse->setEnabled(checked);
    ui->labelReverse->setEnabled(checked);

    ui->cellWidthSpin->setEnabled(checked);
    ui->labelCellWidth->setEnabled(checked);

    on_taskRect_toggled(checked);
}

void LightBuilder::on_taskRect_toggled(bool checked) {
    if(checked && ui->taskEnable->isChecked()) {
        ui->cellHeightSpin->setValue(ui->cellWidthSpin->value());
    }
    ui->cellHeightSpin->setEnabled(!checked && ui->taskEnable->isChecked());
    ui->labelCellHeight->setEnabled(!checked && ui->taskEnable->isChecked());
}

void LightBuilder::on_cellWidthSpin_valueChanged(int arg1) {
    if(ui->taskRect->isChecked()) {
        ui->cellHeightSpin->setValue(arg1);
    }
}

void LightBuilder::on_photonCheckBox_toggled(bool checked) {
    ui->photonDistanceSpin->setEnabled(!checked);
    ui->photonMaxPhotonsSpin->setEnabled(checked);
}

void LightBuilder::on_saveCheck_toggled(bool checked) {
    ui->labelSaveTo->setEnabled(checked);
    ui->pathEdit->setEnabled(checked);
    ui->pathBtn->setEnabled(checked);
}

void LightBuilder::on_cameraTargetCheck_clicked(bool checked) {
    ui->focalLengthLabel->setEnabled(!checked);
    ui->focalLengthSpin->setEnabled(!checked);
}

void LightBuilder::on_renderSettings_visibilityChanged(bool visible) {
    ui->actionRender_Settings->setChecked(visible);
}

void LightBuilder::on_globalIllumination_visibilityChanged(bool visible) {
    ui->actionGlobal_Illumination->setChecked(visible);
}

void LightBuilder::on_cameraSettings_visibilityChanged(bool visible) {
    ui->actionCamera_Settings->setChecked(visible);
}

void LightBuilder::on_workerSettings_visibilityChanged(bool visible) {
    ui->actionWorker_Settings->setChecked(visible);
}

void LightBuilder::on_actionRender_Settings_triggered(bool checked) {
    ui->renderSettings->setVisible(checked);
}

void LightBuilder::on_actionGlobal_Illumination_triggered(bool checked) {
    ui->globalIllumination->setVisible(checked);
}

void LightBuilder::on_actionCamera_Settings_triggered(bool checked) {
    ui->cameraSettings->setVisible(checked);
}

void LightBuilder::on_actionWorker_Settings_triggered(bool checked) {
    ui->workerSettings->setVisible(checked);
}

void LightBuilder::on_pathBtn_clicked() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);

    if(dialog.exec()) {
         ui->pathEdit->setText(dialog.directory().absolutePath());
    }
}

void LightBuilder::on_devicesList_itemChanged(QListWidgetItem *item) {
    Q_UNUSED(item)
    ui->spinMaxThreads->setEnabled(true);
    ui->labelMaxTreads->setEnabled(true);

    for(int i = 0; i < ui->devicesList->count(); i++) {
        if(ui->devicesList->item(i)->checkState() == Qt::Checked) {
            ui->spinMaxThreads->setEnabled(false);
            ui->labelMaxTreads->setEnabled(false);
            break;
        }
    }
}
