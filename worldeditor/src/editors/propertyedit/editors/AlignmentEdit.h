#ifndef ALIGNMENTEDIT_H
#define ALIGNMENTEDIT_H

#include <QWidget>

namespace Ui {
    class AlignmentEdit;
}

class AlignmentEdit : public QWidget {
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
    explicit AlignmentEdit (QWidget *parent = nullptr);
    ~AlignmentEdit ();

    int alignment () const;
    void setAlignment (int value);

signals:
    void alignmentChanged (int value);

private:
    void onToggle ();

    Ui::AlignmentEdit *ui;
};

#endif // ALIGNMENTEDIT_H
