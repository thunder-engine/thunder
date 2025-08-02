#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include <QLocale>

#include <engine.h>

class QTranslator;

class ENGINE_EXPORT EditorSettings : public Object {
    A_OBJECT(EditorSettings, Object, Editor)

    A_METHODS(
        A_SIGNAL(EditorSettings::updated)
    )

public:
    EditorSettings();

    static EditorSettings *instance();
    static void destroy();

    void registerValue(const TString &name, const Variant &value, const TString &annotation = TString());

    Variant value(const TString &name);
    void setValue(const TString &name, const Variant &value);

    void setLanguage(const QLocale &language);

    void setProperty(const char *name, const Variant &value) override;

signals:
    void updated();

public slots:
    void loadSettings();
    void saveSettings();

private:
    TString propertyTag(const TString &hint, const TString &tag) const;

private:
    static EditorSettings *m_pInstance;

    QLocale m_locale;

    QTranslator *m_translator;

};

#endif // EDITORSETTINGS_H
