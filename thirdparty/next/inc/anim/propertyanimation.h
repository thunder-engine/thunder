#ifndef PROPERTYANIMATION_H
#define PROPERTYANIMATION_H

#include "variantanimation.h"

class PropertyAnimationPrivate;

class NEXT_LIBRARY_EXPORT PropertyAnimation : public VariantAnimation {
    A_REGISTER(PropertyAnimation, VariantAnimation, Animation)

public:
    PropertyAnimation               ();

    ~PropertyAnimation              ();

    void                            setTarget                   (Object *object, const char *property);

    Variant                         defaultValue                () const;

    const Object                   *target                      () const;

    const char                     *targetProperty              () const;

    void                            setCurrentValue             (const Variant &value) override;

    void                            setValid                    (bool valid) override;

private:
    PropertyAnimationPrivate       *p_ptr;
};

#endif // PROPERTYANIMATION_H
