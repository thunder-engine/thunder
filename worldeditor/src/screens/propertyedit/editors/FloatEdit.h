#ifndef FLOATEDIT_H
#define FLOATEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class FloatEdit;
}

class FloatEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit FloatEdit(QWidget *parent = nullptr);
    ~FloatEdit();

    Variant data() const override;
    void setData(const Variant &value) override;

    void setEditorHint(const TString &hint) override;

private slots:
    void onValueChanged(int value);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::FloatEdit *ui;

};

#endif // FLOATEDIT_H
