#ifndef LOCALEEDIT_H
#define LOCALEEDIT_H

#include <editor/propertyedit.h>

namespace Ui {
    class LocaleEdit;
}

class LocaleEdit : public PropertyEdit {
    Q_OBJECT

public:
    explicit LocaleEdit(QWidget *parent = nullptr);
    ~LocaleEdit();

private:
    QVariant data() const override;
    void setData(const QVariant &data) override;

private:
    Ui::LocaleEdit *ui;

    static QList<QLocale> m_locales;

};

#endif // LOCALEEDIT_H
