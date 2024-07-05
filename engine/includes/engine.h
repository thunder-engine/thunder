#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <unordered_map>

#include <objectsystem.h>

class Module;
class System;
class File;

class EnginePrivate;

class Actor;
class Scene;
class Component;
class ResourceSystem;
class RenderSystem;
class Resource;
class World;
class PlatformAdaptor;

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
    static Variant value(const std::string &key, const Variant &defaultValue = Variant());

    static void setValue(const std::string &key, const Variant &value);

    static void syncValues();

/*
    Resource management
*/
    static Object *loadResource(const std::string &path);

    static void unloadResource(const std::string &path);
    static void unloadResource(Resource *resource);

    static void reloadResource(const std::string &path);

    template<typename T>
    static T *loadResource(const std::string &path) {
        return dynamic_cast<T *>(loadResource(path));
    }

    static bool isResourceExist(const std::string &path);

    static std::string reference(Object *object);

    static bool reloadBundle();

    static ResourceSystem *resourceSystem();

    static RenderSystem *renderSystem();

/*
    Scene management
*/
    static World *world();

    static Scene *loadScene(const std::string &path, bool additive);

    static void unloadScene(Scene *scene);

    static void unloadAllScenes();

/*
    Misc
*/
    static bool isGameMode();

    static void setGameMode(bool flag);

    static File *file();

    static std::string locationAppDir();

    static std::string locationAppConfig();

    static bool loadTranslator(const std::string &table);

    static std::string translate(const std::string &source);

    static void addModule(Module *module);

    static std::string applicationName();

    static std::string organizationName();

    static void setResource(Object *object, const std::string &uuid);

    static void setPlatformAdaptor(PlatformAdaptor *platform);

    static Actor *composeActor(const std::string &component, const std::string &name, Object *parent = nullptr);

    Object::ObjectList getAllObjectsByType(const std::string &type) const override;

private:
    bool event(Event *event) override;

    static void addSystem(System *system);

};

#endif // ENGINE_H
