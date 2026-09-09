#include "navigation.h"

#include "navigationsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "navigationpanel.h"
#include "property/agenttypeedit.h"

Module *moduleCreate(Engine *engine) {
    return new Navigation(engine);
}
#endif

static const char *meta = \
    "{"
    "   \"module\": \"Navigation\","
    "   \"version\": \"1.0\","
    "   \"description\": \"AI Navigation Module\","
    "   \"author\": \"Evgeniy Prikazchikov\","
    "   \"dependencies\": {"
    "       \"Bullet\": \"module\""
    "   },"
    "   \"objects\": {"
    "       \"NavigationSystem\": \"system\","
    "       \"NavigationPanel\": \"gadget\","
    "       \"AgentTypeEdit\": \"property\""
    "   },"
    "   \"components\": ["
    "       \"NavMeshAgent\","
    "       \"NavMeshLink\","
    "       \"NavMeshObstacle\","
    "       \"NavMeshSurface\""
    "   ]"
    "}";

Navigation::Navigation(Engine *engine) :
        Module(engine),
        m_system(nullptr),
        m_panel(nullptr) {
}

Navigation::~Navigation() {
    delete m_system;
#ifdef SHARED_DEFINE
    delete m_panel;
#endif
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
#ifdef SHARED_DEFINE
    if(strcmp(name, "NavigationPanel") == 0) {
        if(m_panel == nullptr) {
            m_panel = new NavigationPanel();
        }
        return m_panel;
    }
    if(strcmp(name, "AgentTypeEdit") == 0) {
        return new AgentTypeEdit();
    }
#endif
    return nullptr;
}
