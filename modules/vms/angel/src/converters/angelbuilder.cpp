#include "converters/angelbuilder.h"

#include <log.h>
#include <bson.h>

#include <angelscript.h>

#include <QFile>

#include "angelsystem.h"

#define DATA    "Data"

class CBytecodeStream : public asIBinaryStream {
public:
    CBytecodeStream(ByteArray &ptr) :
        array(ptr) {

    }
    int Write(const void *ptr, asUINT size) {
        if( size == 0 ) {
            return size;
        }
        uint32_t offset = array.size();
        array.resize(offset + size);
        memcpy(&array[offset], ptr, size);

        return size;
    }
    int Read(void *ptr, asUINT size) {
        return 0;
    }
protected:
    ByteArray        &array;
};

VariantMap AngelSerial::saveUserData() const {
    VariantMap result;

    result[DATA]  = m_Array;

    return result;
}

AngelBuilder::AngelBuilder() {
    m_pScriptEngine = asCreateScriptEngine();

    m_pScriptEngine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);

    AngelSystem::registerClasses(m_pScriptEngine);
}

bool AngelBuilder::buildProject() {
    m_Outdated = false;
    return true;
}

QString AngelBuilder::builderVersion() {
    return "1.0";
}

uint8_t AngelBuilder::convertFile(IConverterSettings *settings) {
    asIScriptModule *mod = m_pScriptEngine->GetModule("module", asGM_ALWAYS_CREATE);

    QFile file(settings->source());
    if(file.open( QIODevice::ReadOnly)) {
        mod->AddScriptSection("AngelData", file.readAll().data());
        file.close();

        if(mod->Build() >= 0) {
            QFile dst(settings->absoluteDestination());
            if(dst.open( QIODevice::WriteOnly)) {
                AngelSerial serial;
                serial.m_Array.clear();
                CBytecodeStream stream(serial.m_Array);
                mod->SaveByteCode(&stream);

                ByteArray data  = Bson::save( Engine::toVariant(&serial) );

                dst.write((const char *)&data[0], data.size());
                dst.close();
            }
        }
    }

    return 0;
}

void AngelBuilder::messageCallback(const asSMessageInfo *msg, void *param) {
    A_UNUSED(param)
    Log((Log::LogTypes)msg->type) << msg->section << "(line:" << msg->row << "col:" << msg->col << "):" << msg->message;
}
