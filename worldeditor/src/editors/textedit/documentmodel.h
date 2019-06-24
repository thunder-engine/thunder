#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include <QAbstractItemModel>
#include <QPixmap>

class CodeEditor;

class DocumentModel : public QAbstractItemModel {
public:
    DocumentModel();

    CodeEditor *openFile(const QString &path);
    CodeEditor *openFile(const QModelIndex &index);

    void saveAll();

    void closeFile(const QModelIndex &index);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;

private:
    QVariant data(const QModelIndex &index, int role) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    int columnCount(const QModelIndex &) const;

    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent) const;

protected:
    QMap<QString, CodeEditor *> m_Documents;

    QPixmap m_Close;

};

#endif // DOCUMENTMODEL_H
