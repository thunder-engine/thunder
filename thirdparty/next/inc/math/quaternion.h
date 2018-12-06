/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2017 Evgeny Prikazchikov
*/

#ifndef QUATERNION_H_HEADER_INCLUDED
#define QUATERNION_H_HEADER_INCLUDED

#include <global.h>

class Vector3;
class Matrix3;

class NEXT_LIBRARY_EXPORT Quaternion {
public:
    Quaternion                  ();
    Quaternion                  (const Vector3 &dir, areal angle);
    Quaternion                  (const Vector3 &euler);
    Quaternion                  (const Matrix3 &matrix);

    bool                        operator==                  (const Quaternion &quaternion) const;
    bool                        operator!=                  (const Quaternion &quaternion) const;

    Quaternion                  operator*                   (const Quaternion &quaternion) const;
    Vector3                     operator*                   (const Vector3 &vector) const;

    areal                      &operator[]                  (int i);
    areal                       operator[]                  (int i) const;
	
    Quaternion                  inverse                     () const;

    void                        mix                         (const Quaternion &q0, const Quaternion &q1, areal t);
	
    Matrix3                     toMatrix                    () const;
    Vector3                     euler                       () const;
	
    union {
        struct {
            areal x, y, z, w;
        };
        areal q[4];
    };
};

#endif /* QUATERNION_H_HEADER_INCLUDED */
