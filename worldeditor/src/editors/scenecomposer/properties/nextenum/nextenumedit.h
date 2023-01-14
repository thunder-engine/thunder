#ifndef NEXTENUMEDIT_H
#define NEXTENUMEDIT_H

#include <QWidget>

namespace Ui {
    class NextEnumEdit;
}

class NextEnumEdit : public QWidget {
    Q_OBJECT

public:
    explicit NextEnumEdit(QWidget *parent = nullptr);
    ~NextEnumEdit();

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
    Ui::NextEnumEdit *ui;
};

#endif // NEXTENUMEDIT_H
