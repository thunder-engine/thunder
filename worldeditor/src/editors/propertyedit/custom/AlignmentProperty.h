#ifndef ALIGNMENTPROPERTY_H
#define ALIGNMENTPROPERTY_H

#include "Property.h"

class AlignmentProperty : public Property {
    Q_OBJECT
public:
    explicit AlignmentProperty(const QString &name = QString(), QObject *propertyObject = nullptr, QObject *parent = nullptr);

protected slots:
    void onDataChanged(int data);

private:
    QWidget *createEditor(QWidget *parent) const override;

    bool setEditorData(QWidget *editor, const QVariant &data) override;

    QVariant editorData(QWidget *editor) override;

};

#endif // ALIGNMENTPROPERTY_H
