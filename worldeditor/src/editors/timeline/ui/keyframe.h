#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <animationcurve.h>

class TreeRow;

class KeyFrame {
public:
    KeyFrame(AnimationCurve::KeyFrame *key, TreeRow *row);

    AnimationCurve::KeyFrame *key();
    void setKey(AnimationCurve::KeyFrame *key);

    TreeRow *row();

    float position() const;
    void setPosition(float pos);

    float originPosition() const;
    void setOriginPosition(float origin);

    float previousPosition() const;

    bool isSelected();
    void setSelected(bool value);

    bool operator ==(const KeyFrame &left);

private:
    AnimationCurve::KeyFrame *m_key;

    float m_originPosition;

    TreeRow *m_row;

    bool m_selected;
};

#endif // KEYFRAME_H
