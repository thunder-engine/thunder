#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QAbstractItemModel>

class Engine;
class IConverterSettings;

class QFileInfo;

class IAssetEditor {
public:
    IAssetEditor () { }

    virtual ~IAssetEditor () {}

    virtual void loadAsset (IConverterSettings *settings) = 0;

    virtual bool isModified () const = 0;

};

class DocumentModel : public QAbstractItemModel {
public:
    DocumentModel();
    ~DocumentModel();

    void addEditor(uint8_t type, IAssetEditor *editor);

    IAssetEditor *openFile(const QString &path);

    void saveAll();

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

    typedef QMap<int32_t, IAssetEditor *> EditorsMap;
    EditorsMap m_Editors;

};

#endif // DOCUMENTMODEL_H
