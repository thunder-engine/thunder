#include "iplatform.h"

class AndroidPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "android"; }
    bool isPackage() const { return true; }
};
