#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog() Q_DECL_OVERRIDE;

    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void on_pushClipboard_clicked();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
