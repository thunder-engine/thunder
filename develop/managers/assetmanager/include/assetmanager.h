#ifndef ASSETEDITORSMANAGER_H
#define ASSETEDITORSMANAGER_H

#include <QObject>
#include <QMap>
#include <QFileInfo>
#include <QTimer>
#include <QImage>

#include <engine.h>
#include <module.h>

#include "converters/converter.h"

class QFileSystemWatcher;

class ProjectManager;

class IBuilder;

struct Template {
    Template() :
        type(MetaType::INVALID) {

    }
    Template(const QString &p, uint32_t t = MetaType::INVALID) :
        path(p),
        type(t) {

    }

    QString path;
    uint32_t type;
};

Q_DECLARE_METATYPE(Template)

typedef QString FilePath;

Q_DECLARE_METATYPE(FilePath)

class IAssetEditor {
public:
    IAssetEditor            (Engine *engine) :
            m_bModified(false) {
        m_pEngine   = engine;
    }

    virtual ~IAssetEditor() {}

    virtual void            loadAsset           (IConverterSettings *settings) = 0;

    void                    setModified         (bool value) { m_bModified = value; }
    bool                    isModified          () { return m_bModified; }

protected:
    Engine                 *m_pEngine;

    bool                    m_bModified;

};

class AssetManager : public QObject {
    Q_OBJECT
public:
    static AssetManager    *instance            ();

    static void             destroy             ();

    void                    init                (Engine *engine);

    void                    rescan              ();

    void                    addEditor           (uint8_t type, IAssetEditor *editor);
    QObject                *openEditor          (const QFileInfo &source);

    int32_t                 resourceType        (const QFileInfo &source);

    int32_t                 toContentType       (int32_t type);

    void                    removeResource      (const QFileInfo &source);
    void                    renameResource      (const QFileInfo &oldName, const QFileInfo &newName);
    void                    duplicateResource   (const QFileInfo &source);

    void                    makePrefab          (const QString &source, const QFileInfo &target);

    bool                    pushToImport        (const QFileInfo &source);
    bool                    import              (const QFileInfo &source, const QFileInfo &target);

    void                    registerConverter   (IConverter *converter);

    static void             findFreeName        (QString &name, const QString &path, const QString &suff = QString());

    static void             saveSettings        (IConverterSettings *settings);

    string                  guidToPath          (const string &guid);
    string                  pathToGuid          (const string &path);

    QImage                  icon                (const QString &path);

    IConverterSettings     *createSettings      (const QFileInfo &source);


    bool                    isOutdated          () const;

    QString                 artifact            () const;
    void                    setArtifact         (const QString &value);

public slots:
    void                    reimport            ();

signals:
    void                    ready               ();

    void                    directoryChanged    (const QString &path);
    void                    fileChanged         (const QString &path);

    void                    imported            (const QString &path, uint32_t type);
    void                    importStarted       (int count, const QString &stage);
    void                    importFinished      ();

protected slots:
    void                    onPerform           ();

    void                    onFileChanged       (const QString &path, bool force = false);

    void                    onDirectoryChanged  (const QString &path, bool force = false);

private:
    AssetManager            ();
    ~AssetManager           ();

    static AssetManager    *m_pInstance;

protected:
    typedef QMap<int32_t, IAssetEditor *>   EditorsMap;
    EditorsMap              m_Editors;

    typedef QMap<QString, int32_t>          FormatsMap;
    FormatsMap              m_Formats;

    typedef QMap<int32_t, int32_t>          ContentTypeMap;
    ContentTypeMap          m_ContentTypes;

    typedef QMap<QString, IConverter *>     ConverterMap;
    ConverterMap            m_Converters;

    VariantMap              m_Guids;
    VariantMap              m_Paths;

    QFileSystemWatcher     *m_pDirWatcher;
    QFileSystemWatcher     *m_pFileWatcher;

    QList<IConverterSettings *>  m_ImportQueue;

    ProjectManager         *m_pProjectManager;

    QTimer                 *m_pTimer;

    Engine                 *m_pEngine;

    QList<IBuilder *>       m_pBuilders;

    QString                 m_Artifact;

protected:
    void                    cleanupBundle       ();
    void                    dumpBundle          ();

    bool                    isOutdated          (IConverterSettings *settings);

    bool                    pushToImport        (IConverterSettings *settings);

    bool                    convert             (IConverterSettings *settings);
};

#endif // ASSETEDITORSMANAGER_H
