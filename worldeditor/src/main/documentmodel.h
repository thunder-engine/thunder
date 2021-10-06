#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include <QMessageBox>

class Engine;
class AssetEditor;
class AssetConverterSettings;

class QFileInfo;

class DocumentModel : public QAbstractItemModel {
    Q_OBJECT

public:
    DocumentModel();
    ~DocumentModel();

    void addEditor(AssetEditor *editor);

    QString fileName(AssetEditor *editor) const;

    void newFile(AssetEditor *editor);

    AssetEditor *openFile(const QString &path);

    void saveFile(AssetEditor *editor);
    void saveFileAs(AssetEditor *editor);

    bool checkSave(AssetEditor *editor);

    QList<AssetEditor *> documents();

signals:
    void itemSelected(QObject *item);
    void itemUpdated();

public slots:
    void saveAll();

    void closeFile(AssetEditor *editor);

private:
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    int columnCount(const QModelIndex &) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent) const override;

    bool eventFilter(QObject *object, QEvent *event) override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

private slots:
    void onLoadAsset(QString path);

protected:
    QMap<QString, AssetEditor *> m_Documents;

    typedef QMap<QString, AssetEditor *> EditorsMap;
    EditorsMap m_Editors;

};

#endif // DOCUMENTMODEL_H
