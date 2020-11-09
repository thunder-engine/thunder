#ifndef IBUILDER_H
#define IBUILDER_H

#include "converter.h"

#include <QMap>

class QAbstractItemModel;

class NEXT_LIBRARY_EXPORT BuilderSettings : public IConverterSettings {
public:
    BuilderSettings();
private:
    QString typeName() const Q_DECL_OVERRIDE;
};

class NEXT_LIBRARY_EXPORT IBuilder : public IConverter {
    Q_OBJECT
public:
    IBuilder();

    virtual bool buildProject() = 0;

    virtual QString builderVersion() = 0;

    virtual const QString persistentAsset() const { return ""; }
    virtual const QString persistentUUID() const { return ""; }

    QString project() const { return m_Project; }

    void rescanSources(const QString &path);
    virtual bool isEmpty() const;

    bool isOutdated() const { return m_Outdated; }

    virtual QAbstractItemModel *classMap() const;

    uint8_t convertFile(IConverterSettings *) Q_DECL_OVERRIDE;

private:
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;

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

#endif // IBUILDER_H
