#include "math/amath.h"

/*!
    \class Ray
    \brief The Ray class represents a ray in 3D space.
    \since Next 1.0
    \inmodule Math

    Ray is an infinity line starting from \a position pos and going to some \a direction

    \sa Vector3, Plane, AABBox
*/
/*!
    Constructs an identity ray.
    pos at [0, 0, 0] and dir to [0, 0, 1]
*/
Ray::Ray() :
    pos(Vector3()),
    dir(Vector3(0.0f, 0.0f, 1.0f)) {

}
/*!
    Constructs a ray with \a position and \a direction.
*/
Ray::Ray(const Vector3 &position, const Vector3 &direction) :
        pos(position),
        dir(direction) {
}
/*!
    Returns true if this ray is equal to given \a ray; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Ray::operator==(const Ray &ray) const {
    return (dir == ray.dir) && (pos == ray.pos);
}
/*!
    Returns true if this ray is NOT equal to given \a ray; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool Ray::operator!=(const Ray &ray) const {
    return (dir != ray.dir) || (pos != ray.pos);
}
/*!
    Returns true if this ray intersects the given sphere at \a position and \a radius; otherwise returns false.
    Output argument \a pt contain a closest point of intersection.
*/
bool Ray::intersect(const Vector3 &position, areal radius, Vector3 *pt) {
    Vector3 l = position - pos;
    areal tca   = l.dot(dir);
    if(tca < 0) {
        return false;
    }

    areal d2    = l.dot(l) - tca * tca;
    if(d2 > radius * radius) {
        return false;
    }

    if(pt) {
        areal thc   = sqrt(radius * radius - d2);
        areal t0    = tca - thc;
        areal t1    = tca + thc;

        if(t0 < 0.001f) {
            t0      = t1;
        }

        *pt         = pos + dir * t0;
    }

    return true;
}
/*!
    Returns true if this ray intersects the given \a plane; otherwise returns false.
    Output argument \a pt contain a point of intersection. Argument \a back is a flag to use backface culling.
*/
bool Ray::intersect(const Plane &plane, Vector3 *pt, bool back) {
    Vector3 n = plane.normal;
    areal d = dir.dot(n);
    if(d >= 0.0f) {
        if(back) {
            n = -n;
            d = dir.dot(n);
        } else {
            return false;
        }
    }

    areal t = -n.dot(pos - plane.point) / d;
    if(t <= 0.0f) {
        return false;
    }

    if(pt) {
        *pt = pos + dir * t;
    }

    return true;
}
/*!
    Returns true if this ray intersects the given Axis Aligned Bounding \a box; otherwise returns false.
    Output argument \a pt contain a point of intersection.
*/
bool Ray::intersect(const AABBox &box, Vector3 *pt) {
    Vector3 min, max;
    box.box(min, max);

    bool inside = true;
    char quadrant[3];
    Vector3 candidate;

    for(int i = 0; i < 3; i++) {
        if(pos[i] < min[i]) {
            quadrant[i]     = 1;
            candidate[i]    = min[i];
            inside          = false;
        } else if(pos[i] > max[i]) {
            quadrant[i]     = 0;
            candidate[i]    = max[i];
            inside          = false;
        } else	{
            quadrant[i]     = 2;
        }
    }

    if(inside)	{
        if(pt) {
            *pt = pos;
        }
         return true;
    }

    Vector3 maxT;
    for(int i = 0; i < 3; i++) {
        if(quadrant[i] != 2 && dir[i] != 0.0f) {
            maxT[i] = (candidate[i] - pos[i]) / dir[i];
        } else {
            maxT[i] = -1.0f;
        }
    }

    int whichPlane = 0;
    for(int i = 1; i < 3; i++) {
        if(maxT[whichPlane] < maxT[i]) {
            whichPlane = i;
        }
    }

    if(maxT[whichPlane] < 0.0f) {
        return false;
    }

    Vector3 coord;
    for(int i = 0; i < 3; i++) {
        if(whichPlane != i) {
            coord[i] = pos[i] + maxT[whichPlane] * dir[i];
            if(coord[i] < min[i] || coord[i] > max[i]) {
                return false;
            }
        } else {
            coord[i] = candidate[i];
        }
    }
    if(pt) {
        *pt = coord;
    }

    return true;
}
/*!
    Returns true if this ray intersects the given triangle between \a v1, \a v2 and \a v3 points; otherwise returns false.
    Output argument \a pt contain a point of intersection. Argument \a back is a flag to use backface culling.
*/
bool Ray::intersect(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Vector3 *pt, bool back) {
    Vector3 ip;
    if(!intersect(Plane(v1, v2, v3), &ip, back)) {
        return false;
    }

    Vector3 ve0 = v3 - v1;
    Vector3 ve1 = v2 - v1;
    Vector3 ve2 = ip - v1;
    areal dot00 = ve0.dot(ve0);
    areal dot01 = ve0.dot(ve1);
    areal dot02 = ve0.dot(ve2);
    areal dot11 = ve1.dot(ve1);
    areal dot12 = ve1.dot(ve2);
    areal invDenom  = 1.0f / (dot00 * dot11 - dot01 * dot01);
    Vector2 b = Vector2((dot11 * dot02 - dot01 * dot12) * invDenom, (dot00 * dot12 - dot01 * dot02) * invDenom);

    if((b.x >= 0) && (b.y >= 0) && (b.x + b.y <= 1.0f)) {
        if(pt) {
            *pt     = ip;
        }
        return true;
    }
    return false;
}
/*!
    Returns a new Ray object which result of reflection of current ray.
    Reflection calculating by \a normal vector of reflection surface and intersection \a point.
*/
Ray Ray::reflect(const Vector3 &normal, const Vector3 &point) {
    Ray ret(0.0f, 0.0f);

    ret.pos     = point;
    ret.dir     = dir - normal * (2.0f * dir.dot(normal));
    ret.dir.normalize();

    return ret;
}
/*!
    Returns a new Ray object which result of refraction of current ray.
    Refraction calculating by \a normal vector of reflection surface and intersection \a point with \a ior (Index of Refraction).
*/
Ray Ray::refract(const Vector3 &normal, const Vector3 &point, areal ior) {
    Ray ret(0.0f, 0.0f);

    areal theta = normal.dot(dir);
    areal k     = 1.0f - ior * ior * (1.0f - theta * theta);

    ret.pos     = point;
    ret.dir     = dir * ior - normal * (ior * theta + sqrt(k));
    ret.dir.normalize();

    return ret;
}
/*!
    Returns a new Ray object which result of random directed reflection of current ray.
    Diffuse reflection calculating by \a normal vector of reflection surface and intersection \a point. With \a min and \a max constraints.
*/
Ray Ray::diffuse(const Vector3 &normal, const Vector3 &point, areal min, areal max) {
    Ray ret(0.0f, 0.0f);

    areal r1    = 2.0f * PI * RANGE(min, max);
    areal r2    = RANGE(min, max);
    areal r2s   = sqrt(r2);

    Vector3 u = (fabs(normal.x) > .1 ? Vector3(0, 1, 0) : Vector3(1)).cross(normal);
    u.normalize();
    Vector3 v = normal.cross(u);

    ret.pos     = point;
    ret.dir     = u * cos(r1) * r2s + v * sin(r1) * r2s + normal * sqrt(1 - r2);
    ret.dir.normalize();

    return ret;
}
