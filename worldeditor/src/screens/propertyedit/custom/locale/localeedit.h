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
    Variant data() const override;
    void setData(const Variant &data) override;

private:
    Ui::LocaleEdit *ui;

    static QList<QLocale> m_locales;

};

#endif // LOCALEEDIT_H
