#ifndef LIGHTSOURCERT_H
#define LIGHTSOURCERT_H

#include <components/baselight.h>

class ALightSourceRT : public BaseLight {
    A_REGISTER(ALightSourceRT, BaseLight, Scene)

public:
    void backTrace(Vector3 &pos, const Vector3 &n, const int index, Vector3 &result);

protected:
    void setRotation(const Quaternion &value);

protected:
    Vector3 m_Dir;

};

#endif // LIGHTSOURCERT_H
