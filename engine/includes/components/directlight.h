#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class Camera;

class ENGINE_EXPORT DirectLight : public BaseLight {
    A_OBJECT(DirectLight, BaseLight, Components/Lights)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    DirectLight();

    Camera *camera() const;
    void setCamera(Camera *camera);

private:
    void cleanDirty() override;

    int tilesCount() const override;

    int lightType() const override;

    void drawGizmos() override;

private:
    Camera *m_camera;

};

#endif // DIRECTLIGHT_H
