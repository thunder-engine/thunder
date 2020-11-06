#ifndef IPLATFORM_H
#define IPLATFORM_H

#include <QObject>

class IConverter;

class IPlatform : public QObject {
    Q_OBJECT
public:
    virtual QString name() const = 0;
    virtual bool isPackage() const = 0;
    virtual bool isEmbedded() const = 0;

    virtual IConverter *builder() const = 0;
};

#endif // IPLATFORM_H
