#ifndef ACONTROLLER
#define ACONTROLLER

#include <list>

#include <amath.h>

#include "engine.h"

class Camera;
class Scene;

class NEXT_LIBRARY_EXPORT [[deprecated("To be removed in 2018.4")]] IController {
public:
    IController                 ();

    virtual void                update                      ();
/*
    Camera work
*/
    Camera                     *activeCamera                () const;

    void                        setActiveCamera             (Camera *);
/*
    Utils
*/
    virtual void                selectGeometry              (Vector2 &, Vector2 &);

    virtual void                setSelectedObjects          (const list<uint32_t> &);

protected:
    Camera                     *m_pActiveCamera;

    list<uint32_t>              m_ObjectsList;

};

#endif // ACONTROLLER

