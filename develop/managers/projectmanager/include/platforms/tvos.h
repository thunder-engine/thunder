#ifndef TVOS_H
#define TVOS_H

#include "iplatform.h"

class TvOSPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "tvos"; }
    bool isPackage() const { return false; }
};

#endif // TVOS_H
