#ifndef ALIGNMENTEDIT_H
#define ALIGNMENTEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class AlignmentEdit;
}

class AlignmentEdit : public PropertyEdit {
    Q_OBJECT

public:
    enum Alignment {
        Left    = (1<<0),
        Center  = (1<<1),
        Right   = (1<<2),

        Top     = (1<<4),
        Middle  = (1<<5),
        Bottom  = (1<<6)
    };

public:
    explicit AlignmentEdit(QWidget *parent = nullptr);
    ~AlignmentEdit();

    QVariant data() const;
    void setData(const QVariant &data);

private:
    void onToggle();

    Ui::AlignmentEdit *ui;

};

#endif // ALIGNMENTEDIT_H
