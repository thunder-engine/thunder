#ifndef NEXTENUMEDIT_H
#define NEXTENUMEDIT_H

#include <editor/propertyedit.h>

#include <metaenum.h>

namespace Ui {
    class NextEnumEdit;
}

struct Enum {
    Enum() :
        m_object(nullptr),
        m_value(0) {
    }

    QString m_enumName;
    Object *m_object;
    int32_t m_value;

};
Q_DECLARE_METATYPE(Enum);

class NextEnumEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit NextEnumEdit(QWidget *parent = nullptr);
    ~NextEnumEdit();

    QVariant data() const override;
    void setData(const QVariant &data) override;

private slots:
    void onValueChanged(int item);

private:
    Ui::NextEnumEdit *ui;

    Enum m_value;

    MetaEnum m_metaEnum;

};

#endif // NEXTENUMEDIT_H
