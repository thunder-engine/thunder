#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include "components/collider.h"

class btKinematicCharacterController;

class CharacterController : public Collider {
    A_REGISTER(CharacterController, Collider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, height, CharacterController::height, CharacterController::setHeight),
        A_PROPERTY(float, radius, CharacterController::radius, CharacterController::setRadius)
    )
    A_NOMETHODS()

public:
    CharacterController();

    float height() const;
    void setHeight(float height);

    float radius() const;
    void setRadius(float radius);

private:


protected:
    btKinematicCharacterController *m_pCharacter;

    float m_Height;

    float m_Radius;

};

#endif // CHARACTERCONTROLLER_H
