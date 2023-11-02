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

QVariant SettingsManager::value(const char *name, const QVariant &defaultValue) {
    QVariant result  = property(name);
    if(!result.isValid()) {
        result = defaultValue;
    }
    return result;
}

void SettingsManager::setValue(const char *name, const QVariant &value) {
    QVariant current = SettingsManager::value(name);
    if(current != value) {
        setProperty(name, value);
    }
}

void SettingsManager::loadSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariantMap data = settings.value(SETTINGS).toMap();

    blockSignals(true);
    for(QByteArray &it : dynamicPropertyNames()) {
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
        } else if(userType == QMetaType::Bool) {
            setProperty(it, data[it].toBool());
        } else if(userType == QMetaType::Int) {
            setProperty(it, data[it].toInt());
        } else if(userType == QMetaType::Float) {
            setProperty(it, data[it].toFloat());
        } else {
            setProperty(it, data[it].toString());
        }
    }

    blockSignals(false);
    emit updated();
}

void SettingsManager::saveSettings() {
    QVariantMap data;

    for(QByteArray &it : dynamicPropertyNames()) {
        QVariant value = property(it);
        int userType = value.userType();
        if(userType == QMetaType::type("QFileInfo")) {
            data[it] = value.value<QFileInfo>().filePath();
        } else if(userType == QMetaType::type("QColor")) {
            data[it] = value.value<QColor>().name(QColor::HexArgb);
        } else if(userType == QMetaType::type("QLocale")) {
            setLanguage(value.value<QLocale>());
            data[it] = value.value<QLocale>().name();
        } else {
            data[it] = value.toString();
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
