#include "rendersystem.h"

#define SIDES 32

IRenderSystem::IRenderSystem(Engine *engine) : ISystem(engine) {

}

Vector3List IRenderSystem::pointsArc(const Quaternion &rotation, float size, float start, float angle) {
    Vector3List result;
    int sides       = SIDES / 360.0 * angle;
    float theta     = angle / float(sides - 1) * DEG2RAD;
    float tfactor   = tanf(theta);
    float rfactor   = cosf(theta);

    float x = size * cosf(start * DEG2RAD);
    float y = size * sinf(start * DEG2RAD);

    for(int i = 0; i < sides; i++) {
        result.push_back(rotation * Vector3(x, 0, y));

        float tx = -y;
        float ty =  x;

        x += tx * tfactor;
        y += ty * tfactor;

        x *= rfactor;
        y *= rfactor;
    }
    return result;
}
