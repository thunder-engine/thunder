#ifndef NEXTOBJECT_H
#define NEXTOBJECT_H

#include <QObject>

#include <aobject.h>

class ObjectCtrl;
class QMenu;

class NextObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ name WRITE setName DESIGNABLE true USER true)

public:
    explicit                    NextObject              (AObject *data, ObjectCtrl *ctrl, QObject *parent = 0);

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

    void                        buildObject             (AObject *object, const QString &path = QString());

    AObject                    *findChild               (AObject *parent, QStringList &path);

    AObject::ObjectList         m_Objects;

    ObjectCtrl                 *m_pController;
};

#endif // NEXTOBJECT_H
