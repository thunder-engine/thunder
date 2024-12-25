#ifndef SELECTOREDIT_H
#define SELECTOREDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class SelectorEdit;
}

struct SelectorData {
    QString current;

    QString type;

    QStringList values;
};
Q_DECLARE_METATYPE(SelectorData)

class SelectorEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit SelectorEdit(QWidget *parent = nullptr);
    ~SelectorEdit();

    QVariant data() const override;
    void setData(const QVariant &data) override;

private:
    Ui::SelectorEdit *ui;

    mutable SelectorData m_current;
};

#endif // SELECTOREDIT_H
