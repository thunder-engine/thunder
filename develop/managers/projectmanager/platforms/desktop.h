#include "iplatform.h"

#include "qbsbuilder.h"

class DesktopPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "desktop"; }
    bool isPackage() const { return false; }
    bool isEmbedded() const { return false; }

    IConverter *builder() const {return new QbsBuilder(); }
};
