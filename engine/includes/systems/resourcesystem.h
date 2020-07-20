#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"

class Resource;

class ResourceSystemPrivate;

class NEXT_LIBRARY_EXPORT ResourceSystem : public System {
public:
    typedef unordered_map<string, pair<string, string>> DictionaryMap;

public:
    ResourceSystem();
    ~ResourceSystem() override;

    bool init() override;

    const char *name() const override;

    void update(Scene *) override;

    bool isThreadSafe() const override;

    void setResource(Object *object, const string &uuid);

    bool isResourceExist(const string &path);

    Object *loadResource(const string &path);

    void unloadResource(const string &path);

    void unloadResource(Object *resource);

    string reference(Object *object);

    DictionaryMap &indices() const;

private:
    void deleteFromCahe(Object *object);

private:
    ResourceSystemPrivate *p_ptr;
};

#endif // RESOURCESYSTEM_H
