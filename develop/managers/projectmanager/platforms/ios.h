#ifndef IOS_H
#define IOS_H

#include "iplatform.h"

#include "xcodebuilder.h"

class IOSPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "ios"; }
    bool isPackage() const { return false; }
    bool isEmbedded() const { return true; }

    IConverter *builder() const {return new XcodeBuilder(); }
};

#endif // IOS_H
