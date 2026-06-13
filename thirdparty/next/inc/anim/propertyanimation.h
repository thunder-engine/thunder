/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef PROPERTYANIMATION_H
#define PROPERTYANIMATION_H

#include "variantanimation.h"

class NEXT_LIBRARY_EXPORT PropertyAnimation : public VariantAnimation {
    A_OBJECT(PropertyAnimation, VariantAnimation, Animation)

public:
    PropertyAnimation();

    ~PropertyAnimation();

    void setTarget(Object *object, const char *property);

    Variant defaultValue() const;

    const Object *target() const;

    const char *targetProperty() const;

    void setCurrentValue(const Variant &value) override;

    void setValid(bool valid) override;

private:
    Object *m_object;

    MetaProperty m_property;

    Variant m_default;

};

#endif // PROPERTYANIMATION_H
