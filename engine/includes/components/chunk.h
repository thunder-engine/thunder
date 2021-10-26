#ifndef CHUNK_H
#define CHUNK_H

#include <engine.h>

class Resource;

class NEXT_LIBRARY_EXPORT Chunk : public Object {
    A_REGISTER(Chunk, Object, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Chunk();
    ~Chunk();

    Resource *resource() const;
    void setResource(Resource *resource);

private:
    Resource *m_resource;

};

#endif // CHUNK_H
