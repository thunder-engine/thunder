#include "renderprogress.h"
#include "ui_renderprogress.h"

RenderProgress::RenderProgress(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::RenderProgress) {

    ui->setupUi(this);
}

void RenderProgress::onUpdateProgress(float percent, int elapsed) {
    ui->progressBar->setValue(percent);

    ui->labelElapsedTime->setText(timeFormat(elapsed));

    int estimate   = 0;
    if(elapsed > 0)
        estimate   = ((float)elapsed / percent) * 100;
    ui->labelEstimateTime->setText(timeFormat(estimate));
}

QString RenderProgress::timeFormat(const int time) const {
    int s   = time / 1000;
    int m   = s / 60;
    int h   = s / 3600;

    return QString("%1:%2:%3").
                        arg(h % 24, 2, 10, QChar('0')).
                        arg(m % 60, 2, 10, QChar('0')).
                        arg(s % 60, 2, 10, QChar('0'));
}

void RenderProgress::on_stopButton_clicked() {
    emit stop();
}

void RenderProgress::closeEvent(QCloseEvent *ev) {
    on_stopButton_clicked();
}
