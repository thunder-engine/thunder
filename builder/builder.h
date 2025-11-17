#ifndef BUILDER_H
#define BUILDER_H

#include <QObject>
#include <stack>

#include <astring.h>

class Builder : public QObject {
    Q_OBJECT
public:
    Builder();

    void setPlatform(const TString &platform);

    bool package(const TString &target);

public slots:
    void onImportFinished();
    void onBuildSuccessful();

private:
    std::stack<TString> m_platformsToBuild;
};

#endif // BUILDER_H
