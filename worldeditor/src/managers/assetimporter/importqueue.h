#ifndef IMPORTQUEUE_H
#define IMPORTQUEUE_H

#include <QDialog>

#include <stdint.h>
#include <astring.h>

namespace Ui {
    class ImportQueue;
}

class ImportQueue : public QDialog {
    Q_OBJECT

public:
    explicit ImportQueue(QWidget *parent = nullptr);
    ~ImportQueue();

signals:
    void importFinished();

private slots:
    void onProcessed();

    void onStarted(int count, const TString &action);
    void onImportFinished();

private:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::ImportQueue *ui;

};

#endif // IMPORTQUEUE_H
