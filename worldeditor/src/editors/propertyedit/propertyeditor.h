#ifndef QPROPERTYEDITORWIDGET_H
#define QPROPERTYEDITORWIDGET_H

#include <QWidget>

class PropertyModel;
class Property;
class PropertyFilter;

namespace Ui {
    class PropertyEditor;
}

class PropertyEditor : public QWidget {
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parent = nullptr);

    virtual ~PropertyEditor();

    QObject *object() const;

signals:
    void propertyContextMenuRequested(QString property, const QPoint pos);

public slots:
    void onUpdated();

    void onAnimated(bool flag);

    void clear();

    void setObject(QObject *propertyObject);

protected:
    void updatePersistent(const QModelIndex &index);

    void addObject(QObject *propertyObject, const QString &name = QString(), QObject *parent = nullptr);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_treeView_customContextMenuRequested(const QPoint &pos);

private:
    void changeEvent(QEvent *event) override;

    Ui::PropertyEditor *ui;

    PropertyFilter *m_filter;

    bool m_animated;

    QObject *m_propertyObject;

};

#endif
