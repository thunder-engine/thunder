#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <string>
#include <map>

#include "object.h"
#include <objectsystem.h>

#include <file.h>

class IModule;

class EnginePrivate;

class Actor;
class Scene;

class NEXT_LIBRARY_EXPORT Engine : public ObjectSystem {
public:
    Engine                      (IFile *file, int argc, char **argv);
    ~Engine                     ();
/*
    Main system
*/
    bool                        init                        ();

    bool                        start                       ();

    void                        update                      ();
/*
    Settings
*/
    static Variant              value                       (const string &key, const Variant &defaultValue = Variant());

    static void                 setValue                    (const string &key, const Variant &value);
/*
    Resource management
*/
    static Object              *loadResource                (const string &path);

    static void                 unloadResource              (const string &path);

    template<typename T>
    static T                   *loadResource                (const string &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static string               reference                   (Object *object);

    static void                 reloadBundle                ();
/*
    Misc
*/
    void                        addModule                   (IModule *mode);

    Scene                      *scene                       ();

    static IFile               *file                        ();

    static string               locationAppDir              ();

    static string               locationConfig              ();

    static string               locationAppConfig           ();

    string                      applicationName             () const;

    void                        setApplicationName          (const string &name);

    string                      organizationName            () const;

    void                        setOrganizationName         (const string &name);

    static void                 updateScene                 (Object *object);

    static void                 setResource                 (Object *object, string &uuid);

private:
    EnginePrivate              *p_ptr;

};

#endif // ENGINE_H
