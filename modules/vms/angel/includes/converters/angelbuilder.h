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

    bool buildProject () override;

    QString builderVersion () override;

    QStringList suffixes () const override { return {"as"}; }
    uint32_t type () const override { return MetaType::type<AngelScript *>(); }

    uint8_t convertFile (IConverterSettings *settings) override;

    const QString persistentAsset () const override;
    const QString persistentUUID () const override;

protected:
    static void messageCallback (const asSMessageInfo *msg, void *param);

    AngelSystem *m_pSystem;

    asIScriptEngine *m_pScriptEngine;

    QString m_Destination;

};

#endif // AUDIOCONVERTER_H
