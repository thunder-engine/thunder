#include "components/component.h"

#include "components/actor.h"

Component::Component() :
        Object() {

}

void Component::start() {

}

void Component::update() {

}

bool Component::isEnable() const {
    return true;
}

Actor &Component::actor() const {
    return *(static_cast<Actor *>(parent()));
}
