#ifndef QPROPERTYEDITORWIDGET_H
#define QPROPERTYEDITORWIDGET_H

#include <QWidget>

class PropertyModel;
class Property;
class AObject;
class PropertyFilter;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public QWidget {
    Q_OBJECT

public:
    typedef Property* (*UserTypeCB)(const QString &name, QObject *propertyObject, Property *parent);

    explicit PropertyEditor (QWidget *parent = 0);

    virtual ~PropertyEditor ();

    void                    addObject                   (QObject *propertyObject, const QString &name = QString(), QObject *parent = 0);

    void                    setObject                   (QObject *propertyObject);

    void                    registerCustomPropertyCB    (UserTypeCB callback);

    void                    unregisterCustomPropertyCB  (UserTypeCB callback);

public slots:
    void                    onUpdated                   ();

    void                    clear                       ();

protected:
    void                    updatePersistent            (const QModelIndex &index);

private slots:
    void                    on_lineEdit_textChanged     (const QString &arg1);

private:
    Ui::PropertyEditor     *ui;

    PropertyFilter         *m_pFilter;
};

#endif
