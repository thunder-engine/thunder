#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>

#include <object.h>

class ObjectCtrl;
class QMenu;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit                    NextObject              (Object *data, ObjectCtrl *ctrl, QObject *parent = 0);

    QString                     name                    ();
    void                        setName                 (const QString &name);

    QMenu                      *menu                    (const QString &name);

public slots:
    void                        onUpdated               ();

signals:
    void                        updated                 ();

protected slots:
    void                        onDeleteComponent       ();

protected:
    bool                        event                   (QEvent *e);

    void                        buildObject             (Object *object, const QString &path = QString());

    Object                     *findChild               (Object *parent, QStringList &path);

    Object::ObjectList          m_Objects;

    ObjectCtrl                 *m_pController;
};

#endif // NEXTOBJECT_H
