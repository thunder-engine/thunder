#ifndef STRINGEDIT_H
#define STRINGEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class StringEdit;
}

class StringEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit StringEdit(QWidget *parent = nullptr);
    ~StringEdit();

    Variant data() const override;
    void setData(const Variant &data) override;

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::StringEdit *ui;

};

#endif // STRINGEDIT_H
