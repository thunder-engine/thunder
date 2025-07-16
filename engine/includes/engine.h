#ifndef ENGINE_H
#define ENGINE_H

#include <astring.h>
#include <unordered_map>

#include <objectsystem.h>

class Module;
class System;
class File;

class Actor;
class Scene;
class Component;
class ResourceSystem;
class RenderSystem;
class Resource;
class World;
class PlatformAdaptor;
class NativeBehaviour;

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef ENGINE_LIBRARY
        #define ENGINE_EXPORT __declspec(dllexport)
    #else
        #define ENGINE_EXPORT __declspec(dllimport)
    #endif
#else
    #define ENGINE_EXPORT
#endif

class ENGINE_EXPORT Engine : public ObjectSystem {
public:
    Engine(File *file, const char *path);
    ~Engine();

/*
    Main cycle
*/
    static bool init();

    static bool start();

    static void update();

/*
    Settings
*/
    static Variant value(const String &key, const Variant &defaultValue = Variant());

    static void setValue(const String &key, const Variant &value);

    static void syncValues();

/*
    Resource management
*/
    static Object *loadResource(const String &path);

    static void unloadResource(const String &path);
    static void unloadResource(Resource *resource);

    static void reloadResource(const String &path);

    template<typename T>
    static T *loadResource(const String &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static bool isResourceExist(const String &path);

    static String reference(Object *object);

    static bool reloadBundle();

    static ResourceSystem *resourceSystem();

    static RenderSystem *renderSystem();

/*
    Scene management
*/
    static World *world();

    static Scene *loadScene(const String &path, bool additive);

    static void unloadScene(Scene *scene);

    static void unloadAllScenes();

/*
    Misc
*/
    static bool isGameMode();

    static void setGameMode(bool flag);

    static File *file();

    static String locationAppDir();

    static String locationAppConfig();

    static bool loadTranslator(const String &table);

    static String translate(const String &source);

    static void addModule(Module *module);

    static String applicationName();

    static String organizationName();

    static void setResource(Object *object, const String &uuid);

    static void setPlatformAdaptor(PlatformAdaptor *platform);

    static Actor *composeActor(const String &component, const String &name, Object *parent = nullptr);

    Object::ObjectList getAllObjectsByType(const String &type) const override;

    void addNativeBehaviour(NativeBehaviour *native);
    void removeNativeBehaviour(NativeBehaviour *native);

private:
    bool event(Event *event) override;

    static void addSystem(System *system);

};

#endif // ENGINE_H
