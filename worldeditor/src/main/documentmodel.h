#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QObject>
#include <QMap>

#include <object.h>

class AssetEditor;

class DocumentModel : public QObject {
    Q_OBJECT

public:
    DocumentModel();
    ~DocumentModel();

    void addEditor(AssetEditor *editor);

    void newFile(AssetEditor *editor);

    AssetEditor *openFile(const QString &path);

    std::list<AssetEditor *> documents();

signals:
    void updated();

    void itemsSelected(const std::list<QObject *> &items);

    void objectsSelected(const std::list<Object *> &items);

public slots:
    void closeFile(AssetEditor *editor);

private:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onLoadAsset(QString path);

protected:
    typedef std::map<QString, AssetEditor *> EditorsMap;

    std::list<AssetEditor *> m_documents;
    EditorsMap m_editors;

};

#endif // DOCUMENTMODEL_H
