#ifndef SETTINGSBROWSER_H
#define SETTINGSBROWSER_H

#include <QWidget>

class QAbstractItemModel;

namespace Ui {
    class SettingsBrowser;
}

class SettingsBrowser : public QWidget {
    Q_OBJECT

public:
    explicit SettingsBrowser(QWidget *parent = nullptr);
    ~SettingsBrowser();

    void setModel(QObject *model);

signals:
    void commited();
    void reverted();

private slots:
    void onButtonClick();
    void onModelUpdated();

private:
    void changeEvent(QEvent *event);

    Ui::SettingsBrowser *ui;
};

#endif // SETTINGSBROWSER_H
