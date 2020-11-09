#include "iplatform.h"

#include "qbsbuilder.h"

class AndroidPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "android"; }
    bool isPackage() const { return true; }
    bool isEmbedded() const { return true; }

    IConverter *builder() const {return new QbsBuilder(); }
};
