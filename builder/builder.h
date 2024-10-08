#ifndef BUILDER_H
#define BUILDER_H

#include <QObject>
#include <QStack>

class Builder : public QObject {
    Q_OBJECT
public:
    Builder();

    void setPlatform(const QString &platform);

    void package(const QString &target);

public slots:
    void onImportFinished();

private:
    QStack<QString> m_platformsToBuild;
};

#endif // BUILDER_H
