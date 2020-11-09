#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include <QMessageBox>

class Engine;
class IConverterSettings;

class QFileInfo;

class IAssetEditor {
public:
    IAssetEditor () { }

    virtual ~IAssetEditor () {}

    virtual void loadAsset(IConverterSettings *settings) = 0;

    virtual void saveAsset(const QString &path = QString()) { Q_UNUSED(path) };

    virtual bool isModified() const = 0;

    virtual void setModified(bool flag) { Q_UNUSED(flag) };

    virtual QStringList assetTypes() const = 0;
};

class DocumentModel : public QAbstractItemModel {
public:
    DocumentModel();
    ~DocumentModel();

    void addEditor(IAssetEditor *editor);

    QString fileName(IAssetEditor *editor) const;

    IAssetEditor *openFile(const QString &path);

    void saveFile(IAssetEditor *editor);
    void saveFileAs(IAssetEditor *editor);

    void saveAll();

    bool checkSave(IAssetEditor *editor);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

public slots:
    void closeFile(const QString &path);

private:
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    int columnCount(const QModelIndex &) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent) const override;

protected:
    QMap<QString, IAssetEditor *> m_Documents;

    typedef QMap<QString, IAssetEditor *> EditorsMap;
    EditorsMap m_Editors;

};

#endif // DOCUMENTMODEL_H
