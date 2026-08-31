#include "navigation.h"

#include "navigationsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new Navigation(engine);
}
#endif

static const char *meta = \
    "{"
    "   \"module\": \"Navigation\","
    "   \"version\": \"1.0\","
    "   \"description\": \"Navigation Module\","
    "   \"author\": \"Evgeniy Prikazchikov\","
    "   \"dependencies\": ["
    "       \"Bullet\""
    "   ]"
    "   \"objects\": {"
    "       \"NavigationSystem\": \"system\""
    "   },"
    "   \"components\": ["
    "       \"NavigationAgent\","
    "       \"NavigationLink\","
    "       \"NavigationObstacle\""
    "   ]"
    "}";

Navigation::Navigation(Engine *engine) :
    Module(engine),
    m_system(nullptr) {
}

Navigation::~Navigation() {
    delete m_system;
}

const char *Navigation::metaInfo() const {
    return meta;
}

void *Navigation::getObject(const char *name) {
    if(strcmp(name, "NavigationSystem") == 0) {
        if(m_system == nullptr) {
            m_system = new NavigationSystem();
        }
        return m_system;
    }
    return nullptr;
}
