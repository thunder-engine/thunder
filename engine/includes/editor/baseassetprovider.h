#ifndef BASEASSETPROVIDER_H
#define BASEASSETPROVIDER_H

#include <engine.h>

class FileSystemWatcher;

class ENGINE_EXPORT BaseAssetProvider : public Object {
    A_OBJECT(BaseAssetProvider, Object, Core)

    A_METHODS(
        A_SLOT(BaseAssetProvider::onFileChanged),
        A_SLOT(BaseAssetProvider::onDirectoryChanged)
    )

public:
    BaseAssetProvider();
    ~BaseAssetProvider();

    void init(bool force);

    void renameResource(const TString &source, const TString &destination);
    void removeResource(const TString &source);
    void duplicateResource(const TString &source);

public: // slots
    void onFileChanged(const TString &path);
    void onFileChangedForce(const TString &path, bool force = false);

    void onDirectoryChanged(const TString &path);
    void onDirectoryChangedForce(const TString &path, bool force = false);

private:
    FileSystemWatcher *m_dirWatcher;

};

#endif // BASEASSETPROVIDER_H
