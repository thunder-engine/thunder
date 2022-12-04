#ifndef MESHGROUP_H
#define MESHGROUP_H

#include "resource.h"

class Mesh;

class ENGINE_EXPORT MeshGroup : public Resource {
    A_REGISTER(MeshGroup, Resource, Resources)

    A_METHODS(
        A_METHOD(int,  MeshGroup::lodsCount),
        A_METHOD(void, MeshGroup::addLod),
        A_METHOD(Mesh *, MeshGroup::lod),
        A_METHOD(void, MeshGroup::setLod)
    )

public:
    MeshGroup();
    ~MeshGroup();

    int lodsCount() const;

    int addLod(Mesh *lod);

    Mesh *lod(int lod);
    void setLod(int lod, Mesh *data);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    mutable deque<Mesh *> m_lods;

};

#endif // MESHGROUP_H
