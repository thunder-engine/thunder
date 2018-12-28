#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>

#include "converter.h"

#include "resources/angelscript.h"

class asSMessageInfo;
class asIScriptEngine;

class AngelSerial : public AngelScript {
public:
    VariantMap                  saveUserData                () const;
protected:
    friend class AngelConverter;

};

class AngelConverter : public QObject, public IConverter {
    Q_OBJECT
public:
    AngelConverter              ();

    string                      format                      () const { return "as"; }
    uint32_t                    contentType                 () const { return ContentCode; }
    uint32_t                    type                        () const { return MetaType::type<AngelScript *>(); }
    uint8_t                     convertFile                 (IConverterSettings *);

protected:
    static void                 messageCallback             (const asSMessageInfo *msg, void *param);

    asIScriptEngine            *m_pScriptEngine;

};

#endif // AUDIOCONVERTER_H
