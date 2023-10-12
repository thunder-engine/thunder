#ifndef ASSETEDITOR_H
#define ASSETEDITOR_H

#include <QWidget>
#include <QString>

#include <engine.h>

class AssetConverterSettings;

class ENGINE_EXPORT AssetEditor : public QWidget {
    Q_OBJECT

public:
    AssetEditor();
    ~AssetEditor();

    virtual void loadAsset(AssetConverterSettings *settings) = 0;

    virtual void loadData(const Variant &data, const QString &suffix);

    virtual bool isSingleInstance() const;

    virtual AssetEditor *createInstance();

    virtual QStringList suffixes() const = 0;

    const QList<AssetConverterSettings *> &documentsSettings() const;

    bool checkSave();

signals:
    void dropAsset(QString);
    void updateAsset();

    void itemsSelected(QList<QObject *> items);
    void objectsSelected(QList<Object *> objects);

    void updated();

public slots:
    virtual void onActivated();

    virtual void onNewAsset();
    virtual void onSave();
    virtual void onSaveAs();

protected:
    virtual void setModified(bool flag);

    virtual bool isModified() const = 0;

    virtual void saveAsset(const QString &path = QString());

    int closeAssetDialog();

protected:
    QList<AssetConverterSettings *> m_settings;

};

#endif // ASSETEDITOR_H
