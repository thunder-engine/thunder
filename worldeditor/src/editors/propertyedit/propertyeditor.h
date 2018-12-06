#ifndef QPROPERTYEDITORWIDGET_H
#define QPROPERTYEDITORWIDGET_H

#include <QWidget>

class PropertyModel;
class Property;
class Object;
class PropertyFilter;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public QWidget {
    Q_OBJECT

public:
    typedef Property* (*UserTypeCB)(const QString &name, QObject *propertyObject, Property *parent);

    explicit PropertyEditor (QWidget *parent = nullptr);

    virtual ~PropertyEditor ();

    void                    addObject                   (QObject *propertyObject, const QString &name = QString(), QObject *parent = nullptr);

    void                    setObject                   (QObject *propertyObject);

    void                    registerCustomPropertyCB    (UserTypeCB callback);

    void                    unregisterCustomPropertyCB  (UserTypeCB callback);

signals:
    void                    insertKeyframe              (QString &property);

public slots:
    void                    onUpdated                   ();

    void                    onAnimated                  (bool flag);

    void                    clear                       ();

protected:
    void                    updatePersistent            (const QModelIndex &index);

private slots:
    void                    on_lineEdit_textChanged     (const QString &arg1);

    void                    on_treeView_customContextMenuRequested  (const QPoint &pos);

    void                    onInsertKeyframe            ();

private:
    Ui::PropertyEditor     *ui;

    PropertyFilter         *m_pFilter;

    bool                    m_Animated;
};

#endif
