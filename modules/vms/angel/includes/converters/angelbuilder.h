#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>

#include "builder.h"

#include "resources/angelscript.h"

class asSMessageInfo;
class asIScriptEngine;

class AngelSystem;

class AngelSerial : public AngelScript {
public:
    VariantMap saveUserData () const;
protected:
    friend class AngelBuilder;

};

class AngelBuilder : public IBuilder {
    Q_OBJECT
public:
    AngelBuilder (AngelSystem *system);

protected:
    bool buildProject () Q_DECL_OVERRIDE;

    QString builderVersion () Q_DECL_OVERRIDE;

    QStringList suffixes () const Q_DECL_OVERRIDE { return {"as"}; }
    uint32_t type () const Q_DECL_OVERRIDE { return MetaType::type<AngelScript *>(); }

    uint8_t convertFile (IConverterSettings *settings) Q_DECL_OVERRIDE;

    const QString persistentAsset () const Q_DECL_OVERRIDE;
    const QString persistentUUID () const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Angel_Behaviour.as"; }

    static void messageCallback (const asSMessageInfo *msg, void *param);

    AngelSystem *m_pSystem;

    asIScriptEngine *m_pScriptEngine;

    QString m_Destination;

};

#endif // AUDIOCONVERTER_H
