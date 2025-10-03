#include "editorsettings.h"

#include <QVariant>

#include <QSettings>
#include <QColor>
#include <QCoreApplication>
#include <QTranslator>

#define SETTINGS ".Settings"

EditorSettings *EditorSettings::m_pInstance = nullptr;

EditorSettings::EditorSettings() :
        m_translator(new QTranslator()) {

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

void EditorSettings::registerValue(const TString &name, const Variant &value, const TString &annotation) {
    blockSignals(true);
    setProperty(name.data(), value);
    setDynamicPropertyInfo(name.data(), annotation.data());
    blockSignals(false);
}

Variant EditorSettings::value(const TString &name) {
    return property(name.data());
}

void EditorSettings::setValue(const TString &name, const Variant &value) {
    Variant current = EditorSettings::value(name);
    if(current != value) {
        setProperty(name.data(), value);
    }
}

void EditorSettings::loadSettings() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariantMap data = settings.value(SETTINGS).toMap();

    blockSignals(true);
    for(const TString &it : dynamicPropertyNames()) {
        TString info = propertyTag(dynamicPropertyInfo(it.data()), "editor=");
        Variant propertyValue = property(it.data());

        int userType = propertyValue.userType();
        switch(userType) {
            case MetaType::BOOLEAN: {
                bool value = data[it.data()].toBool();
                setProperty(it.data(), value);
            } break;
            case MetaType::INTEGER: {
                int value = data[it.data()].toInt();
                setProperty(it.data(), value);
            } break;
            case MetaType::FLOAT: {
                float value = data[it.data()].toFloat();
                setProperty(it.data(), value);
            } break;
            case MetaType::STRING: {
                TString value(data[it.data()].toString().toStdString());
                setProperty(it.data(), value);
            } break;
            case MetaType::VECTOR4: {
                if(info == "Color") {
                    QString str(data[it.data()].toString());
                    if(!str.isEmpty()) {
                        QColor color(str);
                        Vector4 value(color.redF(), color.greenF(), color.blueF(), color.alphaF());
                        setProperty(it.data(), value);
                    }
                }
            } break;
            default: break;
        }
    }

    blockSignals(false);
}

void EditorSettings::saveSettings() {
    if(isSignalsBlocked()) {
        return;
    }

    QVariantMap data;

    for(const TString &it : dynamicPropertyNames()) {
        Variant propertyValue = property(it.data());

        int userType = propertyValue.userType();
        switch(userType) {
            case MetaType::BOOLEAN: {
                data[it.data()] = propertyValue.toBool();
            } break;
            case MetaType::INTEGER: {
                data[it.data()] = propertyValue.toInt();
            } break;
            case MetaType::FLOAT: {
                data[it.data()] = propertyValue.toFloat();
            } break;
            case MetaType::STRING: {
                data[it.data()] = propertyValue.toString().data();
            } break;
            case MetaType::VECTOR4: {
                Vector4 v = propertyValue.toVector4();
                QString str(QColor::fromRgbF(v.x, v.y, v.z, v.w).name(QColor::HexArgb));

                data[it.data()] = str;
            } break;
            default: break;
        }
    }

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(SETTINGS, data);
}

void EditorSettings::setLanguage(const QLocale &locale) {
    if(m_translator && m_locale != locale) {
        m_locale = locale;
        QCoreApplication::removeTranslator(m_translator);
        m_translator->load(locale, QString(), QString(), ":/Translations");
        QCoreApplication::installTranslator(m_translator);
    }
}

void EditorSettings::setProperty(const char *name, const Variant &value) {
    Object::setProperty(name, value);

    TString editor = propertyTag(dynamicPropertyInfo(name), "editor=");
    if(editor == "Locale") {
        setLanguage(QLocale(value.toString().data()));
    }

    saveSettings();
    updated();
}

void EditorSettings::updated() {
    emitSignal(_SIGNAL(updated()));
}

TString EditorSettings::propertyTag(const TString &hint, const TString &tag) const {
    StringList list(hint.split(','));
    for(TString it : list) {
        int index = it.indexOf(tag);
        if(index > -1) {
            return it.remove(tag);
        }
    }
    return TString();
}
