#ifndef PROPERTY_H
#define PROPERTY_H

#include <QVariant>

#include <engine.h>

class PropertyEdit;

class Property : public QObject {
    Q_OBJECT

public:
    explicit Property(const TString &name, Property *parent, bool root);

    void setPropertyObject(Object *propertyObject);

    TString name() const;

    TString editorHints() const;
    void setEditorHints(const TString &hints);

    QWidget *getEditor(QWidget *parent) const;
    PropertyEdit *editor() const;

    bool isRoot() const;
    bool isReadOnly() const;

    Variant value() const;
    void setValue(const Variant &value);

    void updateEditor();

    QSize sizeHint(const QSize &size) const;

    bool isCheckable() const;
    bool isChecked() const;
    void setChecked(bool value);

    static TString editorName(const TString &hints, const TString &typeName);

    static TString propertyTag(const TString &hints, const TString &tag);
    static bool hasTag(const TString &hints, const TString &tag);

    static void trimmType(TString &type, bool &isArray);

signals:
    void propertyChanged(const Object::ObjectList &objects, const TString &property, Variant value);

protected slots:
    void onDataChanged();
    void onEditorDestoyed();

protected:
    PropertyEdit *createEditor(QWidget *parent) const;

protected:
    Object *m_nextObject;

    TString m_hints;
    TString m_name;

    mutable TString m_typeNameTrimmed;

    mutable PropertyEdit *m_editor;

    bool m_root;
    bool m_checkable;

    bool m_readOnly;
};

#endif // PROPERTY_H
