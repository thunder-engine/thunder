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
    explicit Property(const TString &name, Property *parent, bool root);

    void setPropertyObject(Object *propertyObject);

    TString name() const;
    void setName(const TString &value);

    TString editorHints() const;
    void setEditorHints(const TString &hints);

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

    static TString propertyTag(const TString &hints, const TString &tag);
    static bool hasTag(const TString &hints, const TString &tag);

    static void trimmType(TString &type, bool &isArray);

    static QVariant qVariant(const Variant &value, const TString &typeName, const TString &hints, Object *object);
    static QVariant qObjectVariant(const Variant &value, const TString &typeName, const TString &editor, Object *object);

signals:
    void propertyChanged(const Object::ObjectList &objects, const TString &property, Variant value);

protected slots:
    void onDataChanged();
    void onEditorDestoyed();

protected:
    virtual QWidget *createEditor(QWidget *parent) const;

    Variant aVariant(const QVariant &value, uint32_t type, const TString &typeName);
    Variant aObjectVariant(const QVariant &value, uint32_t type, const TString &typeName);

protected:
    Object *m_nextObject;

    TString m_hints;
    TString m_name;
    mutable TString m_typeNameTrimmed;

    mutable QWidget *m_editor;

    bool m_root;
    bool m_checkable;

    bool m_readOnly;
};

#endif // PROPERTY_H
