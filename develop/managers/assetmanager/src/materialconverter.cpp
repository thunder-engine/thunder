#include "materialconverter.h"

#include <QFile>

#include <bson.h>

#include <file.h>

#include "common.h"
#include "shaderbuilder.h"

#include "projectmanager.h"

uint8_t MaterialConverter::convertFile(IConverterSettings *settings) {
    ShaderBuilder *builder  = new ShaderBuilder;
    builder->load(settings->source());
    if(builder->build()) {
        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( builder->object() );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}
