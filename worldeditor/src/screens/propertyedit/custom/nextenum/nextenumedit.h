#ifndef NEXTENUMEDIT_H
#define NEXTENUMEDIT_H

#include <editor/propertyedit.h>

#include <metaenum.h>

namespace Ui {
    class NextEnumEdit;
}

class NextEnumEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit NextEnumEdit(QWidget *parent = nullptr);
    ~NextEnumEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

    void setEnumData(const TString &name, Object *object);

private slots:
    void onValueChanged(int item);

private:
    Ui::NextEnumEdit *ui;

    TString m_enumName;

    Object *m_object;

    int32_t m_value;

    MetaEnum m_metaEnum;

};

#endif // NEXTENUMEDIT_H
