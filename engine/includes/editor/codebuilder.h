#ifndef CODEBUILDER_H
#define CODEBUILDER_H

#include "assetconverter.h"

#include <QMap>

class QAbstractItemModel;

class NEXT_LIBRARY_EXPORT BuilderSettings : public AssetConverterSettings {
public:
    BuilderSettings();
private:
    QString typeName() const Q_DECL_OVERRIDE;
};

class NEXT_LIBRARY_EXPORT CodeBuilder : public AssetConverter {
    Q_OBJECT
public:
    CodeBuilder();

    virtual bool buildProject() = 0;

    virtual QString builderVersion() = 0;

    virtual const QString persistentAsset() const;
    virtual const QString persistentUUID() const;

    virtual QStringList platforms() const;

    QString project() const;

    QStringList sources() const;

    void rescanSources(const QString &path);
    virtual bool isEmpty() const;
    virtual bool isPackage(const QString &platform) const;

    bool isOutdated() const;

    virtual QAbstractItemModel *classMap() const;

    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

signals:
    void buildSuccessful();

private:
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    void renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName) Q_DECL_OVERRIDE;

protected:
    void updateTemplate(const QString &src, const QString &dst, QStringMap &values);

    void generateLoader(const QString &dst, const QStringList &modules);

    QString formatList(const QStringList &list) const;

protected:
    QStringMap m_Values;

    QString m_Project;

    QStringList m_Sources;

    bool m_Outdated;
};

#endif // CodeBUILDER_H
