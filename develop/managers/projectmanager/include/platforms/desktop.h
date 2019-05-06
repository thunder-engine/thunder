#include "iplatform.h"

class DesktopPlatform : public IPlatform {
    Q_OBJECT
public:
    QString name() const { return "desktop"; }
    bool isPackage() const { return false; }
};
