#ifndef BOOLEANEDIT_H
#define BOOLEANEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class BooleanEdit;
}

class BooleanEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit BooleanEdit(QWidget *parent = nullptr);
    ~BooleanEdit();

    QVariant data() const;
    void setData(const QVariant &data);

private:
    Ui::BooleanEdit *ui;

};

#endif // BOOLEANEDIT_H
