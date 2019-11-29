#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>

#include <object.h>

class QMenu;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit                    NextObject              (Object *data, QObject *parent = nullptr);

    QString                     name                    ();
    void                        setName                 (const QString &name);

    QMenu                      *menu                    (Object *obj);

    Object                     *component               (const QString &name);

    Object                     *findChild               (QStringList &path);

public slots:
    void                        onUpdated               ();

signals:
    void                        aboutToBeChanged        (Object *object, const QString &property, const Variant &value);
    void                        changed                 (Object *object, const QString &property);

    void                        updated                 ();

    void                        deleteComponent         (const QString &name);

protected slots:
    void                        onDeleteComponent       ();

protected:
    bool                        event                   (QEvent *e);

    QString                     editor                  (MetaProperty &property);

    void                        buildObject             (Object *object, const QString &path = QString());

    Object                     *m_pObject;

};

#endif // NEXTOBJECT_H
