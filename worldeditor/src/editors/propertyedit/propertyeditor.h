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
    typedef Property*(*UserTypeCB)(const QString &name, QObject *propertyObject, Property *parent);

    explicit PropertyEditor(QWidget *parent = nullptr);

    virtual ~PropertyEditor();

    void registerCustomPropertyCB(UserTypeCB callback);

    void unregisterCustomPropertyCB(UserTypeCB callback);

signals:
    void propertyContextMenuRequested(QString property, const QPoint pos);

public slots:
    void onUpdated();

    void onAnimated(bool flag);

    void clear();

    QObject *object() const;
    void setObject(QObject *propertyObject);

protected:
    void updatePersistent(const QModelIndex &index);

    void addObject(QObject *propertyObject, const QString &name = QString(), QObject *parent = nullptr);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    void changeEvent(QEvent *event) override;

    Ui::PropertyEditor     *ui;

    PropertyFilter         *m_pFilter;

    bool                    m_Animated;

    QObject                *m_pPropertyObject;
};

#endif
