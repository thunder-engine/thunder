#include "platform.h"

#include "qbsbuilder.h"

class DesktopPlatform : public Platform {
    Q_OBJECT
public:
    QString name() const { return "desktop"; }
    bool isPackage() const { return false; }
    bool isEmbedded() const { return false; }

    AssetConverter *builder() const {return new QbsBuilder(); }
};
