#ifndef PROJECTSETTINGS_H
#define PROJECTSETTINGS_H

#include <QWidget>

namespace Ui {
    class ProjectSettings;
}

class ProjectSettings : public QWidget {
    Q_OBJECT
public:
    explicit ProjectSettings(QWidget *parent = 0);
    ~ProjectSettings();

private slots:
    void onSettingsUpdated();

private:
    void changeEvent(QEvent *event) override;

    Ui::ProjectSettings *ui;

};

#endif // PROJECTSETTINGS_H
