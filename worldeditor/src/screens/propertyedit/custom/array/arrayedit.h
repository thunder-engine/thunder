#ifndef ARRAYEDIT_H
#define ARRAYEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class ArrayEdit;
}

class ArrayElement;

class ArrayEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit ArrayEdit(QWidget *parent = nullptr);
    ~ArrayEdit();

    QVariant data() const override;
    void setData(const QVariant &data) override;

    void setObject(QObject *object, const QString &name) override;

protected:
    void addOne();

private slots:
    void onAddItem();
    void onRemoveItem();
    void onCountChanged();

    void onDataChanged();
    void onEditFinished();
    void onDeleteElement();

private:
    Ui::ArrayEdit *ui;

    QVariantList m_list;

    QString m_propertyName;

    QList<ArrayElement *> m_editors;

    int m_height;

    bool m_dynamic;

};

#endif // ARRAYEDIT_H
