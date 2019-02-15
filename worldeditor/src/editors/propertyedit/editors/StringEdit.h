#ifndef STRINGEDIT_H
#define STRINGEDIT_H

#include <QWidget>

namespace Ui {
    class StringEdit;
}

class StringEdit : public QWidget {
    Q_OBJECT

public:
    explicit StringEdit     (QWidget *parent = 0);
    ~StringEdit             ();

    void                    setText         (const QString &text);
    QString                 text            () const;

signals:
    void                    editFinished    ();

private:
    Ui::StringEdit         *ui;
};

#endif // STRINGEDIT_H
