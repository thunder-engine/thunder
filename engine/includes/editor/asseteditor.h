#ifndef ASSETEDITOR_H
#define ASSETEDITOR_H

#include <QWidget>
#include <QString>

#include <engine.h>

class AssetConverterSettings;

class NEXT_LIBRARY_EXPORT AssetEditor : public QWidget {
    Q_OBJECT

public:
    AssetEditor();
    ~AssetEditor();

    virtual void newAsset();

    virtual void loadAsset(AssetConverterSettings *settings) = 0;

    virtual void saveAsset(const QString &path = QString());

    virtual bool isModified() const = 0;

    virtual bool isSingleInstance() const;

    virtual AssetEditor *createInstance();

    virtual void setModified(bool flag);

    virtual QStringList suffixes() const = 0;

signals:
    void dropAsset(QString);
    void updateAsset();

    void itemSelected(QObject *item);
    void itemUpdated();

protected:
    AssetConverterSettings *m_pSettings;

};

#endif // ASSETEDITOR_H
