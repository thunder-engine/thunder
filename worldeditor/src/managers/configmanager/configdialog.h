#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
    class ConfigDialog;
}

class IBuilder;

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog   (QWidget *parent = nullptr);
    ~ConfigDialog           ();

protected:
    bool                    checkQbsVersion                     ();

private slots:
    void on_pushOK_clicked();

private:
    Ui::ConfigDialog       *ui;

    IBuilder               *m_pBuilder;
};

#endif // CONFIGDIALOG_H
