#ifndef ARRAYELEMENT_H
#define ARRAYELEMENT_H

#include <QWidget>
#include <astring.h>
#include <variant.h>

class PropertyEdit;

namespace Ui {
    class ArrayElement;
}

class ArrayElement : public QWidget {
    Q_OBJECT

public:
    explicit ArrayElement(QWidget *parent = nullptr);
    ~ArrayElement();

    Variant data() const;
    void setData(int index, const Variant &data, const TString &editor, Object *object, const TString &typeName);

    int32_t index() const;

signals:
    void dataChanged();
    void editFinished();
    void deleteElement();

private:
    Ui::ArrayElement *ui;

    TString m_tag;

    PropertyEdit *m_editor;

    int32_t m_index;

};

#endif // ARRAYELEMENT_H
