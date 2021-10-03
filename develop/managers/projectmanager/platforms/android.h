#include "platform.h"

#include "qbsbuilder.h"

class AndroidPlatform : public Platform {
    Q_OBJECT
public:
    QString name() const { return "android"; }
    bool isPackage() const { return true; }
    bool isEmbedded() const { return true; }

    AssetConverter *builder() const {return new QbsBuilder(); }
};
