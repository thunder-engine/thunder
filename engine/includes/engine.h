#ifndef ENGINE_H
#define ENGINE_H

#include <astring.h>
#include <objectsystem.h>

class Module;
class System;

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
    Engine(const char *path);
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
    static Variant value(const TString &key, const Variant &defaultValue = Variant());

    static void setValue(const TString &key, const Variant &value);

    static void syncValues();

/*
    Resource management
*/
    static Object *loadResource(const TString &path);

    static void unloadResource(const TString &path);
    static void unloadResource(Resource *resource);

    static void reloadResource(const TString &path);

    template<typename T>
    static T *loadResource(const TString &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static bool isResourceExist(const TString &path);

    static TString reference(Object *object);

    static bool reloadBundle();

    static ResourceSystem *resourceSystem();

    static RenderSystem *renderSystem();

/*
    Scene management
*/
    static World *world();

    static Scene *loadScene(const TString &path, bool additive);

    static void unloadScene(Scene *scene);

    static void unloadAllScenes();

/*
    Misc
*/
    static bool isGameMode();

    static void setGameMode(bool flag);

    static TString locationAppDir();

    static TString locationAppConfig();

    static bool loadTranslator(const TString &table);

    static TString translate(const TString &source);

    static void addModule(Module *module);

    static TString applicationName();

    static TString organizationName();

    static void setResource(Object *object, const TString &uuid);

    static bool setPlatformAdaptor(PlatformAdaptor *platform);

    static Actor *composeActor(const TString &component, const TString &name, Object *parent = nullptr);

    Object::ObjectList getAllObjectsByType(const TString &type) const override;

    void addNativeBehaviour(NativeBehaviour *native);
    void removeNativeBehaviour(NativeBehaviour *native);

private:
    bool event(Event *event) override;

    static void addSystem(System *system);

};

#endif // ENGINE_H
