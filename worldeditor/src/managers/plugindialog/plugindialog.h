#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include <QDialog>

#include <engine.h>

class PluginManager;
class QSortFilterProxyModel;

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
    void on_restartButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

private:
    void changeEvent(QEvent *event) override;

    void updatePersistent(const QModelIndex &index);

protected:
    Ui::PluginDialog *ui;

    QSortFilterProxyModel *m_filter;

};

#endif // PLUGINDIALOG_H
