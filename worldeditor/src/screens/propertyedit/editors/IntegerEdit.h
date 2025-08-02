#ifndef INTEGEREDIT_H
#define INTEGEREDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class IntegerEdit;
}

class IntegerEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit IntegerEdit(QWidget *parent = nullptr);
    ~IntegerEdit();

    QVariant data() const override;
    void setData(const QVariant &data) override;

    void setEditorHint(const TString &hint) override;

private slots:
    void onValueChanged(int value);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::IntegerEdit *ui;

};

#endif // INTEGEREDIT_H
