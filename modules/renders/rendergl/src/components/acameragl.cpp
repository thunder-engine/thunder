#include "components/acameragl.h"

#include <avariant.h>

#include "agl.h"

#include <components/actor.h>

void ACameraGL::setShaderParams(uint32_t program) {
    int location;
    location    = glGetUniformLocation(program, "camera.target");
    if(location > -1) {
        Vector3 t;
        glUniform4f(location, t.x, t.y, t.z, m_Far);
    }
    location	= glGetUniformLocation(program, "camera.position");
    if(location > -1) {
        Vector3 p = actor().position();
        glUniform4f(location, p.x, p.y, p.z, m_Near);
    }
}
