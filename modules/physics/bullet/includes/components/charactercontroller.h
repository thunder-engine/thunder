#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

#include "collider.h"

class btKinematicCharacterController;
class btPairCachingGhostObject;

class BULLET_EXPORT CharacterController : public Collider {
    A_OBJECT(CharacterController, Collider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, slopeLimit, CharacterController::slopeLimit, CharacterController::setSlopeLimit),
        A_PROPERTY(float, stepOffset, CharacterController::stepOffset, CharacterController::setStepOffset),
        A_PROPERTY(float, skinWidth, CharacterController::skinWidth, CharacterController::setSkinWidth),
        A_PROPERTY(Vector3, center, CharacterController::center, CharacterController::setCenter),
        A_PROPERTY(float, height, CharacterController::height, CharacterController::setHeight),
        A_PROPERTY(float, radius, CharacterController::radius, CharacterController::setRadius)
    )
    A_METHODS(
        A_METHOD(void, CharacterController::move),
        A_METHOD(bool, CharacterController::isGrounded)
    )

public:
    CharacterController();
    ~CharacterController();

    float height() const;
    void setHeight(float height);

    float radius() const;
    void setRadius(float radius);

    float slopeLimit() const;
    void setSlopeLimit(float width);

    float stepOffset() const;
    void setStepOffset(float height);

    float skinWidth() const;
    void setSkinWidth(float width);

    Vector3 center() const;
    void setCenter(const Vector3 center);

    Vector3 gravity() const;
    void setGravity(const Vector3 gravity);

    void move(const Vector3 &direction);

    bool isGrounded() const;

private:
    void createCollider() override;

    btCollisionShape *shape() override;

    void update() override;

    void destroyCharacter();

    void drawGizmosSelected() override;

protected:
    Vector3 m_center;

    btKinematicCharacterController *m_character;

    btPairCachingGhostObject *m_ghostObject;

    float m_height;

    float m_radius;

    float m_skin;

    float m_slope;

    float m_step;

    bool m_dirty;

};
typedef CharacterController* CharacterControllerPtr;

#endif // CHARACTERCONTROLLER_H
