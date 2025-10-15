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

    Variant data() const override;
    void setData(const Variant &data) override;

    void setObject(Object *object, const TString &name) override;

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

    VariantList m_list;

    std::list<ArrayElement *> m_editors;

    TString m_typeName;
    TString m_propertyName;
    TString m_editorName;

    int m_height;

    int m_metaType;

    bool m_dynamic;

};

#endif // ARRAYEDIT_H
