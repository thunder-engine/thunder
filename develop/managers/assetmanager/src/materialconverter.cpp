#include "materialconverter.h"

#include <QFile>

#include <abson.h>

#include <file.h>

#include "common.h"
#include "shaderbuilder.h"

#include "projectmanager.h"

uint8_t MaterialConverter::convertFile(IConverterSettings *settings) {
    ShaderBuilder *builder  = new ShaderBuilder;
    builder->load(settings->source());
    if(builder->build()) {
        QFile file(ProjectManager::instance()->importPath() + "/" + settings->destination());
        if(file.open(QIODevice::WriteOnly)) {
            AByteArray data = ABson::save( builder->object() );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}
