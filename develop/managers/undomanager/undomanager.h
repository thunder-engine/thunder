#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <QObject>
#include <QString>
#include <QStack>
#include <QUndoCommand>

#include <memory>

#include <engine.h>

class Object;
class ObjectCtrl;

class UndoManager : public QUndoStack {
    Q_OBJECT

public:
    static UndoManager         *instance            ();

    static void                 destroy             ();

    void                        init                ();

private:
    UndoManager                 () {}
    ~UndoManager                () {}

    static UndoManager         *m_pInstance;
};



#endif // UNDOMANAGER_H
