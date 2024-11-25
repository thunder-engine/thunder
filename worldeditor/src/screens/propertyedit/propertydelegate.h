#ifndef PROPERTYDELEGATE_H
#define PROPERTYDELEGATE_H

#include <QStyledItemDelegate>

class QSignalMapper;

class PropertyDelegate : public QStyledItemDelegate {
public:
    explicit PropertyDelegate(QObject *parent = nullptr);
    virtual ~PropertyDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void parseEditorHints(QWidget *editor, const QString &editorHints) const;

    QSignalMapper *m_finishedMapper;

};

#endif // PROPERTYDELEGATE_H
