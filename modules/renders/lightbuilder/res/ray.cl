struct ARay {
    float3              pos;
    float3              dir;
};

struct ARay castRay(float x, float y, __global struct camera_data *camera) {
    float ratio = camera->pos.w;
    float fov   = tan(camera->eye.w * (PI / 180.0f));
    float3 dir  = normalize(camera->pos.xyz - camera->eye.xyz);

    float3 right    = cross(dir, camera->up.xyz);
    float3 up       = cross(right, dir);
    float3 view     = (float3)( (x - 0.5f) * ratio * fov) * right +
                      (float3)(-(y - 0.5f) * fov) * up +
                      camera->eye.xyz + dir;

    struct ARay result;

    result.pos  = camera->eye.xyz;
    result.dir  = normalize(view - camera->eye.xyz);

    return result;
}

struct ARay reflect(const struct ARay *ray, const float3 n, const float3 p) {
    struct ARay ret;

    ret.pos     = p;
    ret.dir     = normalize(ray->dir - n * (float3)(2.0 * dot(ray->dir, n)));

    return ret;
}

struct ARay refract(const struct ARay *ray, const float3 n, const float3 p, float c0, float c1) {
    struct ARay ret;

    float eta   = c0 / c1;
    float theta = dot(ray->dir, n);
    float k     = 1.0f - eta * eta * (1.0 - theta * theta);

    ret.pos     = p;
    ret.dir     = normalize(ray->dir * eta - n * (eta * theta + sqrt(k)));

    return ret;
}

struct ARay diffuse(const float3 n, const float3 p, uint2 *seed) {
    struct ARay ret;

    float4 r    = rand(seed);

    float r1    = 2.0f * PI * r.x;
    float r2    = r.y;
    float r2s   = sqrt(r2);

    float3 u    = normalize(cross((fabs(n.x) > 0.1f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f)), n));
    float3 v    = cross(n, u);

    ret.pos     = p;
    ret.dir     = normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + n * sqrt(1 - r2));

    return ret;
}

bool intersect_sphere(const struct ARay *ray, const float3 p, float r, float3 *pt) {
    float3 l    = p - ray->pos;
    float tca   = dot(l, ray->dir);
    if(tca < 0) {
        return false;
    }

    float d2    = dot(l, l) - tca * tca;
    if(d2 > r * r) {
        return false;
    }

    if(pt) {
        float thc   = sqrt(r * r - d2);
        float t0    = tca - thc;
        float t1    = tca + thc;

        if(t0 < 0.001) {
            t0  = t1;
        }

        *pt     = ray->pos + ray->dir * t0;
    }

    return true;
}

bool intersect_plane(const struct ARay *ray, struct APlane *p, float3 *pt, bool back) {
    float d = dot(ray->dir, p->normal);
    if(d >= 0.0f) {
        if(back) {
            p->normal   = -p->normal;
            d   = dot(ray->dir, p->normal);
        } else {
            return false;
        }
    }

    //float t = dot(normal, normal * p->d - ray->pos) / d;
    float t = dot(-p->normal, ray->pos - p->point) / d;
    if(t <= 0.0) {
        return false;
    }

    if(pt) {
        *pt = ray->pos + ray->dir * t;
    }
    return true;
}

bool intersect_triangle(const struct ARay *ray, const float3 v1, const float3 v2, const float3 v3, float3 *pt, bool back) {
    float3 ip;
    struct APlane plane = plane_set_points(v1, v2, v3);
    if(!intersect_plane(ray, &plane, &ip, back)) {
        return false;
    }
    float3 ve0      = v3 - v1;
    float3 ve1      = v2 - v1;
    float3 ve2      = ip - v1;
    float dot00     = dot(ve0, ve0);
    float dot01     = dot(ve0, ve1);
    float dot02     = dot(ve0, ve2);
    float dot11     = dot(ve1, ve1);
    float dot12     = dot(ve1, ve2);

    float invDenom  = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float2 b        = (float2)((dot11 * dot02 - dot01 * dot12) * invDenom, (dot00 * dot12 - dot01 * dot02) * invDenom);

    if((b.x >= 0) && (b.y >= 0) && (b.x + b.y <= 1.0f)) {
        if(pt) {
            *pt     = ip;
        }
        return true;
    }

    return false;
}

bool intersect_aabb(const struct ARay *ray, float3 bmin, float3 bmax, float3 *pt) {
    bool inside     = true;
    float pos[3]    = {ray->pos.x, ray->pos.y, ray->pos.z};
    float dir[3]    = {ray->dir.x, ray->dir.y, ray->dir.z};
    float min[3]    = {bmin.x, bmin.y, bmin.z};
    float max[3]    = {bmax.x, bmax.y, bmax.z};
    char quadrant[3];
    float candidate[3];

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
            *pt = (float3)(pos[0], pos[1], pos[2]);
        }
         return true;
    }

    float maxT[3];
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

    float coord[3];
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
        *pt = (float3)(coord[0], coord[1], coord[2]);
    }

    return true;
}
