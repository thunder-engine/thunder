#ifndef RENDERPROGRESS_H
#define RENDERPROGRESS_H

#include <QMainWindow>

namespace Ui {
    class RenderProgress;
}

class RenderProgress : public QMainWindow {
    Q_OBJECT
public:
    RenderProgress          (QWidget *parent = 0);
    
signals:
    void                    stop                            ();

public slots:
    void                    onUpdateProgress                (float percent, int elapsed);
    
private slots:
    void                    on_stopButton_clicked           ();

private:
    QString                 timeFormat                      (const int time) const;

    void                    closeEvent(QCloseEvent *ev);

private:
    Ui::RenderProgress     *ui;

};

#endif // RENDERPROGRESS_H
