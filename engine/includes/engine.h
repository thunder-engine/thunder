#ifndef ENGINE_H
#define ENGINE_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include <objectsystem.h>

#include "file.h"

class Module;

class EnginePrivate;

class Scene;
class System;

class NEXT_LIBRARY_EXPORT Engine : public ObjectSystem {
public:
    Engine                      (File *file, const char *path);
    ~Engine                     ();
/*
    Main system
*/
    bool                        init                        ();

    bool                        start                       ();

    void                        resize                      ();

    void                        update                      ();
/*
    Settings
*/
    static Variant              value                       (const string &key, const Variant &defaultValue = Variant());

    static void                 setValue                    (const string &key, const Variant &value);

    static void                 syncValues                  ();
/*
    Resource management
*/
    static Object              *loadResource                (const string &path);

    static void                 unloadResource              (const string &path, bool force = false);

    template<typename T>
    static T                   *loadResource                (const string &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static bool                 isResourceExist             (const string &path);

    static string               reference                   (Object *object);

    static void                 reloadBundle                ();

    System                     *resourceSystem              () const;

/*
    Misc
*/
    static bool                 isGameMode                  ();

    static void                 setGameMode                 (bool flag);

    void                        addModule                   (Module *module);

    Scene                      *scene                       ();

    static File                *file                        ();

    static string               locationAppDir              ();

    static string               locationAppConfig           ();

    static bool                 loadTranslator              (const string &table);

    static string               translate                   (const string &source);

    string                      applicationName             () const;

    string                      organizationName            () const;

    void                        updateScene                 (Scene *scene);

    static void                 setResource                 (Object *object, const string &uuid);

    void                        processEvents               ();

private:
    bool                        event                       (Event *event) override;

private:
    EnginePrivate              *p_ptr;

};

#endif // ENGINE_H
