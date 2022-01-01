#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>

#include <engine.h>

class PluginManager;

namespace Ui {
    class PluginDialog;
}

class PluginDialog : public QDialog  {
    Q_OBJECT

public:
    PluginDialog(QWidget *parent = nullptr);
    ~PluginDialog();

public slots:
    void on_loadButton_clicked();

private slots:
    void on_closeButton_clicked();

private:
    void changeEvent(QEvent *event) override;

    void updatePersistent(const QModelIndex &index);

protected:
    Ui::PluginDialog *ui;
};

#endif // PLUGINDIALOG_H
