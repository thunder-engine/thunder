#ifndef TVOS_H
#define TVOS_H

#include "ios.h"

class TvOSPlatform : public IOSPlatform {
    Q_OBJECT
public:
    QString name() const { return "tvos"; }
};

#endif // TVOS_H
