#ifndef IOS_H
#define IOS_H

#include "platform.h"

#include "xcodebuilder.h"

class IOSPlatform : public Platform {
    Q_OBJECT
public:
    QString name() const { return "ios"; }
    bool isPackage() const { return false; }
    bool isEmbedded() const { return true; }

    AssetConverter *builder() const {return new XcodeBuilder(); }
};

#endif // IOS_H
