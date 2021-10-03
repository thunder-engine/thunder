#ifndef PLATFORM_H
#define PLATFORM_H

#include <QObject>

class AssetConverter;

class Platform : public QObject {
    Q_OBJECT
public:
    virtual QString name() const = 0;
    virtual bool isPackage() const = 0;
    virtual bool isEmbedded() const = 0;

    virtual AssetConverter *builder() const = 0;
};

#endif // PLATFORM_H
