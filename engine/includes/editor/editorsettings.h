#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include <QObject>
#include <QLocale>

#include <engine.h>

class QTranslator;

class ENGINE_EXPORT EditorSettings : public QObject {
    Q_OBJECT

public:
    static EditorSettings *instance();

    static void destroy();

    QVariant value(const char *name, const QVariant &defaultValue = QVariant());
    void setValue(const char *name, const QVariant &value);

    void setLanguage(const QLocale &language);

signals:
    void updated();

public slots:
    void loadSettings();
    void saveSettings();

private:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    EditorSettings();

    static EditorSettings *m_pInstance;

    QTranslator *m_translator;
    QLocale m_locale;

};

#endif // EDITORSETTINGS_H
