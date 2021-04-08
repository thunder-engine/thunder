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
    void addItem(const QString &text, const QVariant &data);

    void clear();

    int findText(const QString &text);
    int findData(const QVariant &data);

    void setCurrentIndex(int index);

    QString currentText() const;
    QVariant currentData() const;

signals:
    void currentIndexChanged(const QString &);

private:
    Ui::ComboEdit *ui;
};

#endif // COMBOEDIT_H
