#ifndef COMBOEDIT_H
#define COMBOEDIT_H

#include <QWidget>

namespace Ui {
    class ComboEdit;
}

class ComboEdit : public QWidget {
    Q_OBJECT

public:
    explicit ComboEdit(QWidget *parent = nullptr);
    ~ComboEdit();

    void addItems(const QStringList &items);

    int findText(const QString &text);

    void setCurrentIndex(int index);

    QString currentText() const;

signals:
    void currentIndexChanged(const QString &);

private:
    Ui::ComboEdit *ui;
};

#endif // COMBOEDIT_H
