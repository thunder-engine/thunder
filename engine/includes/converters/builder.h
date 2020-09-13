#ifndef IBUILDER_H
#define IBUILDER_H

#include "converter.h"

#include <QMap>

class QAbstractItemModel;

class NEXT_LIBRARY_EXPORT IBuilder : public IConverter {
    Q_OBJECT
public:
    IBuilder();

    virtual bool buildProject() = 0;

    virtual QString builderVersion() = 0;

    virtual uint32_t contentType() const { return IConverter::ContentCode; }
    virtual uint32_t type() const;
    virtual uint8_t convertFile(IConverterSettings *);

    virtual const QString persistentAsset() const { return ""; }
    virtual const QString persistentUUID() const { return ""; }

    QString project() const { return m_Project; }

    void rescanSources(const QString &path);
    bool isEmpty() const;

    bool isOutdated() const { return m_Outdated; }

    virtual QAbstractItemModel *classMap() const;

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
