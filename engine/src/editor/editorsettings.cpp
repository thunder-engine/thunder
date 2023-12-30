#include "editorsettings.h"

#include <QVariant>

#include <QSettings>
#include <QFileInfo>
#include <QColor>
#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>

#include <QEvent>

#define SETTINGS ".Settings"

EditorSettings *EditorSettings::m_pInstance = nullptr;

EditorSettings::EditorSettings() :
        m_translator(new QTranslator()) {

    installEventFilter(this);

    connect(this, &EditorSettings::updated, this, &EditorSettings::saveSettings);
}

EditorSettings *EditorSettings::instance() {
    if(!m_pInstance) {
        m_pInstance = new EditorSettings;
    }
    return m_pInstance;
}

void EditorSettings::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void EditorSettings::registerProperty(const char *name, const QVariant &value) {
    int32_t index = dynamicPropertyNames().indexOf(name);
    if(index == -1) {
        blockSignals(true);
        setProperty(name, value);
        blockSignals(false);
    }
}

QVariant EditorSettings::value(const char *name, const QVariant &defaultValue) {
    QVariant result  = property(name);
    if(!result.isValid()) {
        result = defaultValue;
    }
    return result;
}

void EditorSettings::setValue(const char *name, const QVariant &value) {
    QVariant current = EditorSettings::value(name);
    if(current != value) {
        setProperty(name, value);
    }
}

void EditorSettings::loadSettings() {
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
}

void EditorSettings::saveSettings() {
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
        } else if(userType == QMetaType::Bool) {
            data[it] = value.toBool();
        } else if(userType == QMetaType::Int) {
            data[it] = value.toInt();
        } else if(userType == QMetaType::Float) {
            data[it] = value.toFloat();
        } else {
            data[it] = value.toString();
        }
    }

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(SETTINGS, data);
}

bool EditorSettings::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::DynamicPropertyChange) {
        emit updated();
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void EditorSettings::setLanguage(const QLocale &locale) {
    if(m_translator && m_locale != locale) {
        m_locale = locale;
        QCoreApplication::removeTranslator(m_translator);
        m_translator->load(locale, QString(), QString(), ":/Translations");
        QCoreApplication::installTranslator(m_translator);
    }
}
