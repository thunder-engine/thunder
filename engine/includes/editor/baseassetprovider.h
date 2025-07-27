#ifndef BASEASSETPROVIDER_H
#define BASEASSETPROVIDER_H

#include <QObject>

#include <engine.h>

class QFileSystemWatcher;

class ENGINE_EXPORT BaseAssetProvider : public QObject {
public:
    BaseAssetProvider();

    ~BaseAssetProvider();

    void init();

    void renameResource(const QString &source, const QString &destination);
    void removeResource(const QString &source);
    void duplicateResource(const QString &source);

    void cleanupBundle();

protected:
    bool copyRecursively(const QString &sourceFolder, const QString &destFolder);

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
