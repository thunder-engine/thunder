#ifndef ARRAYELEMENT_H
#define ARRAYELEMENT_H

#include <QWidget>
#include <astring.h>

class PropertyEdit;

namespace Ui {
    class ArrayElement;
}

class ArrayElement : public QWidget {
    Q_OBJECT

public:
    explicit ArrayElement(QWidget *parent = nullptr);
    ~ArrayElement();

    QVariant data() const;
    void setData(int index, const QVariant &data, const TString &name);

    int32_t index() const;

signals:
    void dataChanged();
    void editFinished();
    void deleteElement();

private:
    Ui::ArrayElement *ui;

    PropertyEdit *m_editor;

    int32_t m_index;

};

#endif // ARRAYELEMENT_H
