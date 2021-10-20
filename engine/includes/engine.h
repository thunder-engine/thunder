#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <unordered_map>

#include <objectsystem.h>

#include "file.h"

class Module;

class EnginePrivate;

class Actor;
class Scene;
class Chunk;
class System;
class PlatformAdaptor;

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

    static void                 unloadResource              (const string &path);

    static void                 reloadResource              (const string &path);

    template<typename T>
    static T                   *loadResource                (const string &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static bool                 isResourceExist             (const string &path);

    static string               reference                   (Object *object);

    static bool                 reloadBundle                ();

    static System              *resourceSystem              ();

/*
    Scene management
*/
    static Scene               *scene                       ();

    static Chunk               *loadSceneChunk              (const string &path, bool additive);

    static void                 unloadSceneChunk            (Chunk *chunk);

/*
    Misc
*/
    static bool                 isGameMode                  ();

    static void                 setGameMode                 (bool flag);

    static File                *file                        ();

    static string               locationAppDir              ();

    static string               locationAppConfig           ();

    static bool                 loadTranslator              (const string &table);

    static string               translate                   (const string &source);

    static void                 addModule                   (Module *module);

    static string               applicationName             ();

    static string               organizationName            ();

    static void                 setResource                 (Object *object, const string &uuid);

    static void                 setPlatformAdaptor          (PlatformAdaptor *platform);

    static Actor               *composeActor                (const string &component, const string &name, Object *parent = nullptr);

private:
    bool                        event                       (Event *event) override;

    void                        processEvents               () override;

private:
    EnginePrivate              *p_ptr;

};

#endif // ENGINE_H
