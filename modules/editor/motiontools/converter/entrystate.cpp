#include "entrystate.h"

Vector2 EntryState::defaultSize() const {
    return Vector2(170.0f, 40.0f);
}

Vector4 EntryState::color() const {
    return Vector4(0.22f, 0.46, 0.11f, 1.0f);
}

bool EntryState::isState() const {
    return true;
}
