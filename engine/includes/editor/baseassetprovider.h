#ifndef BASEASSETPROVIDER_H
#define BASEASSETPROVIDER_H

#include <QObject>

#include <engine.h>

class QFileSystemWatcher;

class ENGINE_EXPORT BaseAssetProvider : public QObject {
    Q_OBJECT
public:
    BaseAssetProvider();

    ~BaseAssetProvider();

    void init();

    void renameResource(const TString &source, const TString &destination);
    void removeResource(const TString &source);
    void duplicateResource(const TString &source);

    void cleanupBundle();

protected:
    bool copyRecursively(const TString &sourceFolder, const TString &destFolder);

public slots:
    void onFileChanged(const QString &path);
    void onFileChangedForce(const QString &path, bool force = false);

    void onDirectoryChanged(const QString &path);
    void onDirectoryChangedForce(const QString &path, bool force = false);

private:
    QFileSystemWatcher *m_dirWatcher;
    QFileSystemWatcher *m_fileWatcher;

};

#endif // BASEASSETPROVIDER_H
