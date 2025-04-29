#ifndef CODEBUILDER_H
#define CODEBUILDER_H

#include "assetconverter.h"

#include <QMap>

class QAbstractItemModel;

class ENGINE_EXPORT CodeBuilder : public AssetConverter {
    Q_OBJECT
public:
    CodeBuilder();

    virtual bool buildProject() = 0;

    virtual QString builderVersion() = 0;

    virtual bool isNative() const = 0;

    virtual const QString persistentAsset() const;
    virtual const QString persistentUUID() const;

    virtual QStringList platforms() const;

    QString project() const;

    QStringList sources() const;

    void rescanSources(const QString &path);
    virtual bool isEmpty() const;
    virtual bool isBundle(const QString &platform) const;

    void makeOutdated();
    bool isOutdated() const;

    virtual QAbstractItemModel *classMap() const;

    ReturnCode convertFile(AssetConverterSettings *) override;

signals:
    void buildSuccessful();

private:
    AssetConverterSettings *createSettings() override;

    void renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName) override;

protected:
    void updateTemplate(const QString &src, const QString &dst);

    void generateLoader(const QString &dst, const QStringList &modules);

    QString formatList(const QStringList &list) const;

protected:
    QMap<QString, QString> m_values;

    QString m_project;

    QStringList m_sources;

    bool m_outdated;

};

class ENGINE_EXPORT BuilderSettings : public AssetConverterSettings {
public:
    explicit BuilderSettings(CodeBuilder *builder);

    CodeBuilder *builder() const;

private:
    QStringList typeNames() const override;

    QString defaultIconPath(const QString &) const override;

    bool isCode() const override;

private:
    CodeBuilder *m_builder;

};

#endif // CODEBUILDER_H
