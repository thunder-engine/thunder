#ifndef CODEBUILDER_H
#define CODEBUILDER_H

#include "assetconverter.h"

class QAbstractItemModel;

class ENGINE_EXPORT CodeBuilder : public AssetConverter {
public:
    CodeBuilder();

    virtual bool buildProject() = 0;

    virtual bool isNative() const = 0;

    virtual const TString persistentAsset() const;
    virtual const TString persistentUUID() const;

    virtual StringList platforms() const;

    TString project() const;

    StringList sources() const;

    void rescanSources(const TString &path);
    virtual bool isEmpty() const;
    virtual bool isBundle(const TString &platform) const;

    void makeOutdated();
    bool isOutdated() const;

    virtual QAbstractItemModel *classMap() const;

    ReturnCode convertFile(AssetConverterSettings *) override;

    void buildSuccessful();

private:
    AssetConverterSettings *createSettings() override;

    void renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName) override;

protected:
    void updateTemplate(const TString &src, const TString &dst);

    void generateLoader(const TString &dst, const StringList &modules);

    TString formatList(const StringList &list) const;

protected:
    std::map<TString, TString> m_values;

    TString m_project;

    std::set<TString> m_sources;

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
