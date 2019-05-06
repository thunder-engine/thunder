#include "settingsmanager.h"

#include <QVariant>

#include <QSettings>
#include <QFileInfo>

#define SETTINGS ".Settings"

SettingsManager *SettingsManager::m_pInstance = nullptr;

SettingsManager *SettingsManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new SettingsManager;
    }
    return m_pInstance;
}

void SettingsManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}
#include <QDebug>
void SettingsManager::loadSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariantMap data = settings.value(SETTINGS).toMap();

    for(QByteArray it : dynamicPropertyNames()) {
        QVariant value = property(it);
        int userType = value.userType();
        if(userType == QMetaType::type("QFileInfo")) {
            qDebug() << data[it].toString();
            setProperty(it, QVariant::fromValue(QFileInfo(data[it].toString())));
        }
    }
    emit updated();
}

void SettingsManager::saveSettings() {
    QVariantMap data;

    for(QByteArray it : dynamicPropertyNames()) {
        QVariant value = property(it);
        int userType = value.userType();
        if(userType == QMetaType::type("QFileInfo")) {
            qDebug() << value.value<QFileInfo>().filePath();
            data[it] = value.value<QFileInfo>().filePath();
        }
    }

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(SETTINGS, data);
}
