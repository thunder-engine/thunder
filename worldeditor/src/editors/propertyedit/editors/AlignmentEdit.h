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
        Left = 0,
        Center,
        Right
    };

public:
    explicit AlignmentEdit(QWidget *parent = nullptr);
    ~AlignmentEdit();

    Alignment alignment() const;
    void setAlignment(Alignment value);

signals:
    void alignmentChanged(int value);

private:
    void onToggle();

    Ui::AlignmentEdit *ui;
};

#endif // ALIGNMENTEDIT_H
