#ifndef MESHGL_H
#define MESHGL_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

class AMeshGL : public Mesh {
    A_OVERRIDE(AMeshGL, Mesh, Resources)
public:
    typedef vector<uint32_t *>  BufferVector;

    BufferVector                vbuffer;
    BufferVector                ibuffer;

public:
    AMeshGL                     ();

    ~AMeshGL                    ();

    void                        loadUserData        (const AVariantMap &data);

    bool                        isVBO               ()  { return mVBO; }

protected:
    void                        clear               ();

    bool                        mVBO;

};

#endif // MESHGL_H

//void                        attach              (mesh_data *ready_mesh, joint_data *ready_array, mesh_data *attach_mesh, joint_data *attach_array, uint8_t proxy);
//void                        detach              ();
//void                        cpu_calculation     (mesh_instance_data *instance, joint_data *pArray);
//void                        load_animation      (char *filename, char *name, mesh_data *pMesh, uint8_t index, unsigned short type, Vector3 *aabb);
