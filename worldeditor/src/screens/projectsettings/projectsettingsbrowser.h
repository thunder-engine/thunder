#ifndef PROJECTSETTINGSBROWSER_H
#define PROJECTSETTINGSBROWSER_H

#include <QWidget>

namespace Ui {
    class ProjectSettingsBrowser;
}

class ProjectSettingsBrowser : public QWidget {
    Q_OBJECT
public:
    explicit ProjectSettingsBrowser(QWidget *parent = 0);
    ~ProjectSettingsBrowser();

public slots:
    void onSettingsUpdated();

private:
    void changeEvent(QEvent *event) override;

    Ui::ProjectSettingsBrowser *ui;

};

#endif // PROJECTSETTINGSBROWSER_H
