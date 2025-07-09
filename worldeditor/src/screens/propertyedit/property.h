#ifndef PROPERTY_H
#define PROPERTY_H

#include <QVariant>

#include <engine.h>

class QWidget;

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

Q_DECLARE_METATYPE(Alignment)
Q_DECLARE_METATYPE(Axises)

class Property : public QObject {
    Q_OBJECT

public:
    explicit Property(const QString &name, Property *parent, bool root);

    void setPropertyObject(QObject *propertyObject);
    void setPropertyObject(Object *propertyObject);

    QString name() const;
    void setName(const QString &value);

    QString editorHints() const;
    void setEditorHints(const QString &hints);

    QWidget *getEditor(QWidget *parent) const;
    QWidget *editor() const;

    virtual bool isRoot() const;
    virtual bool isReadOnly() const;

    virtual QVariant value(int role = Qt::UserRole) const;
    virtual void setValue(const QVariant &value);

    virtual QVariant editorData(QWidget *editor);

    virtual bool setEditorData(QWidget *editor, const QVariant &data);

    virtual QSize sizeHint(const QSize &size) const;

    virtual bool isCheckable() const;
    virtual bool isChecked() const;
    virtual void setChecked(bool value);

    QString propertyTag(const QString &tag) const;
    bool hasTag(const QString &tag) const;

signals:
    void propertyChanged(std::list<Object *> objects, const QString property, Variant value);

protected slots:
    void onDataChanged();
    void onEditorDestoyed();

protected:
    virtual QWidget *createEditor(QWidget *parent) const;

    QVariant qVariant(const Variant &value, const MetaProperty &property, Object *object) const;
    QVariant qObjectVariant(const Variant &value, const std::string &typeName, const QString &editor) const;

    Variant aVariant(const QVariant &value, const Variant &current, const MetaProperty &property);
    Variant aObjectVariant(const QVariant &value, uint32_t type, const std::string &typeName);

protected:
    QObject *m_propertyObject;
    Object *m_nextObject;

    QString m_hints;
    QString m_name;

    mutable QWidget *m_editor;

    bool m_root;
    bool m_checkable;

    bool m_readOnly;
};

#endif // PROPERTY_H
