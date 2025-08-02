#ifndef PROJECTSETTINGSBROWSER_H
#define PROJECTSETTINGSBROWSER_H

#include <QWidget>

#include <engine.h>

namespace Ui {
    class ProjectSettingsBrowser;
}

class ProjectSettingsBrowser : public QWidget {
    Q_OBJECT

public:
    explicit ProjectSettingsBrowser(QWidget *parent = 0);
    ~ProjectSettingsBrowser();

    void init();

private slots:
    void onSettingsUpdated(const Object::ObjectList &list, const TString &property, Variant value);

private:
    void changeEvent(QEvent *event) override;

    Ui::ProjectSettingsBrowser *ui;

};

#endif // PROJECTSETTINGSBROWSER_H
