#ifndef IOS_H
#define IOS_H

#include "iplatform.h"

class IOSPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "ios"; }
    bool isPackage() const { return false; }
};

#endif // IOS_H
