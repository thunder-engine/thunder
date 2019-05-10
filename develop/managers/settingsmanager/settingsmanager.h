#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>

class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager      *instance                   ();

    static void                 destroy                     ();

signals:
    void                        updated                     ();

public slots:
    void                        loadSettings                ();
    void                        saveSettings                ();

private:
    bool                        eventFilter                 (QObject *obj, QEvent *event);

private:
    SettingsManager             ();

    static SettingsManager     *m_pInstance;

};

#endif // SETTINGSMANAGER_H
