#include "keyframe.h"
#include "treerow.h"

KeyFrame::KeyFrame(AnimationCurve::KeyFrame *key, TreeRow *row) :
    m_key(key),
    m_originPosition(0.0f),
    m_row(row),
    m_selected(false) {

}

AnimationCurve::KeyFrame *KeyFrame::key() {
    return m_key;
}

void KeyFrame::setKey(AnimationCurve::KeyFrame *key) {
    m_key = key;
}

TreeRow *KeyFrame::row() {
    return m_row;
}

float KeyFrame::position() const {
    if(m_key) {
        return m_key->m_position;
    }
    return -1.0f;
}

void KeyFrame::setPosition(float pos) {
    if(m_key) {
        m_key->m_position = pos;
        m_row->timelineItem()->update();
        if(m_row->parentRow() && m_row->parentRow()->timelineItem()) {
            m_row->parentRow()->timelineItem()->update();
        }
    }
}

float KeyFrame::originPosition() const {
    return m_originPosition;
}

void KeyFrame::setOriginPosition(float origin) {
    m_originPosition = origin;
}

bool KeyFrame::isSelected() {
    return m_selected;
}

void KeyFrame::setSelected(bool value) {
    m_selected = value;
}

bool KeyFrame::operator ==(const KeyFrame &left) {
    return (m_key == left.m_key && m_selected == left.m_selected);
}
