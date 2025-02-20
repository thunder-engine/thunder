/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef PROPERTYANIMATION_H
#define PROPERTYANIMATION_H

#include "variantanimation.h"

class NEXT_LIBRARY_EXPORT PropertyAnimation : public VariantAnimation {
    A_REGISTER(PropertyAnimation, VariantAnimation, Animation)

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
