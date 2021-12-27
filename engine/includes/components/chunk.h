#ifndef CHUNK_H
#define CHUNK_H

#include <engine.h>

class Resource;
class Component;

class NEXT_LIBRARY_EXPORT Chunk : public Object {
    A_REGISTER(Chunk, Object, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Component *, Chunk::componentInChild)
    )

public:
    Chunk();
    ~Chunk();

    Resource *resource() const;
    void setResource(Resource *resource);

    Component *componentInChild(const string type);

private:
    mutable Resource *m_resource;

};

#endif // CHUNK_H
