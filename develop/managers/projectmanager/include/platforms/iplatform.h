#ifndef IPLATFORM_H
#define IPLATFORM_H

#include <QObject>

class IPlatform : public QObject {
    Q_OBJECT
public:
    virtual QString name() const = 0;
    virtual bool isPackage() const = 0;
};

#endif // IPLATFORM_H
