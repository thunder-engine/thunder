#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include <QObject>
#include <QString>
#include <QStack>

#include <memory>

#include <engine.h>

#include <patterns/asingleton.h>

class Object;
class ObjectCtrl;

class UndoManager : public QObject, public ASingleton<UndoManager> {
    Q_OBJECT

public:
    class IUndoUnit {
    public:
        virtual ~IUndoUnit      () {}

        virtual void            undo                (bool redo) {Q_UNUSED(redo);}

        QString                 name                () const { return m_Name; }

        virtual bool            isValid             () const { return false; }

    protected:
        QString                 m_Name;

    };
    typedef shared_ptr<IUndoUnit>   Undo;

public:
    void                        init                ();

    void                        undo                ();

    void                        redo                ();

    void                        clear               ();

    QString                     undoTop             ();

    QString                     redoTop             ();

    void                        undoClear           ();

    void                        redoClear           ();

    void                        push                (IUndoUnit *unit, bool redo = false, bool override = true);

signals:
    void                        updated             ();

public:
    class UndoObject : public IUndoUnit {
    public:
        UndoObject              (ObjectCtrl *ctrl, const QString &name) {
            m_pController   = ctrl;
            m_Name          = name;
        }
    protected:
        ObjectCtrl             *m_pController;
    };

    class SelectObjects : public UndoObject {
    public:
        SelectObjects           (const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = tr("Selection Change"));
        void                    undo                (bool redo);
        void                    forceUndo           ();
        virtual bool            isValid             () const;
    protected:
        list<uint32_t>          m_Objects;
    };

    class CreateObjects : public UndoObject {
    public:
        CreateObjects           (const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = tr("Create Objects"));
        void                    undo                (bool redo);
        virtual bool            isValid             () const;
    protected:
        QStringList             m_Objects;
        SelectObjects          *m_pSelect;
    };

    class DestroyObjects : public UndoObject {
    public:
        DestroyObjects          (const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = tr("Destroy Objects"));
        void                    undo                (bool redo);
        virtual bool            isValid             () const;
    protected:
        string                  m_Dump;
        QStringList             m_Parents;
    };

    class ParentingObjects : public UndoObject {
    public:
        ParentingObjects        (const Object::ObjectList &objects, Object::ObjectList &, ObjectCtrl *ctrl, const QString &name = tr("Parenting Objects"));
        void                    undo                (bool redo);
        virtual bool            isValid             () const;
    protected:
        QStringList             m_Objects;
        QStringList             m_Parents;
    };

    class PropertyObjects : public UndoObject {
    public:
        PropertyObjects         (const Object::ObjectList &objects, ObjectCtrl *ctrl, const QString &name = tr("Change Property"));
        void                    undo                (bool redo);
        virtual bool            isValid             () const;
    protected:
        string                  m_Dump;
    };

protected:
    friend class ASingleton<UndoManager>;

    typedef QStack<IUndoUnit *> UndoStack;

    UndoStack                   m_UndoStack;

    UndoStack                   m_RedoStack;

protected:
    UndoManager                 () {}

};



#endif // UNDOMANAGER_H
