#include "settingsmanager.h"

#include <QVariant>

#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QColor>
#include <QCoreApplication>
#include <QTranslator>

#include <QEvent>

#define SETTINGS ".Settings"

SettingsManager *SettingsManager::m_pInstance = nullptr;

SettingsManager::SettingsManager() :
    m_Translator(new QTranslator()) {
    installEventFilter(this);
}

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

void SettingsManager::registerProperty(const char *name, const QVariant &value) {
    int32_t index = dynamicPropertyNames().indexOf(name);
    if(index == -1) {
        setProperty(name, value);
    }
}

void SettingsManager::loadSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariantMap data = settings.value(SETTINGS).toMap();

    blockSignals(true);
    for(QByteArray it : dynamicPropertyNames()) {
        QVariant value = property(it);
        int userType = value.userType();
        if(userType == QMetaType::type("QFileInfo")) {
            setProperty(it, QVariant::fromValue(QFileInfo(data[it].toString())));
        } else if(userType == QMetaType::type("QColor")) {
            QString name = data[it].toString();
            if(!name.isEmpty()) {
                setProperty(it, QVariant::fromValue(QColor(name)));
            }
        } else if(userType == QMetaType::type("QLocale")) {
            QLocale locale(data[it].toString());
            setLanguage(locale);
            setProperty(it, locale);
        }
    }

    blockSignals(false);
    emit updated();
}

void SettingsManager::saveSettings() {
    QVariantMap data;

    for(QByteArray it : dynamicPropertyNames()) {
        QVariant value = property(it);
        int userType = value.userType();
        if(userType == QMetaType::type("QFileInfo")) {
            data[it] = value.value<QFileInfo>().filePath();
        } else if(userType == QMetaType::type("QColor")) {
            data[it] = value.value<QColor>().name(QColor::HexArgb);
        } else if(userType == QMetaType::type("QLocale")) {
            setLanguage(value.value<QLocale>());
            data[it] = value.value<QLocale>().name();
        }
    }

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(SETTINGS, data);
}

bool SettingsManager::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::DynamicPropertyChange) {
        emit updated();
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void SettingsManager::setLanguage(const QLocale &locale) {
    if(m_Translator && m_Locale != locale) {
        m_Locale = locale;
        QCoreApplication::removeTranslator(m_Translator);
        m_Translator->load(locale, QString(), QString(), ":/Translations");
        QCoreApplication::installTranslator(m_Translator);
    }
}
