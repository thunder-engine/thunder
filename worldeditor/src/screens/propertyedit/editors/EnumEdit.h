#ifndef ENUMEDIT_H
#define ENUMEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class EnumEdit;
}

class EnumEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit EnumEdit(QWidget *parent = nullptr);
    ~EnumEdit();

    QVariant data() const override;
    void setData(const QVariant &data) override;

    void setObject(QObject *object, const QString &name) override;

private:
    Ui::EnumEdit *ui;

};

#endif // ENUMEDIT_H
