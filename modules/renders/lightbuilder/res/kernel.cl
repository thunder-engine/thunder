typedef enum tag_mesh_types {
    MESH_SPHERE         = 1,
    MESH_PLANE          = 2
} mesh_types;

typedef enum tag_collect_types {
    BACK_TRACING        = 1,
    PHOTON_MAPPING      = 2,
    PATH_TRACING        = 3
} collect_types;

typedef enum tag_task_state_types {
    TASK_WAIT           = 1,
    TASK_PROGRESS,
    TASK_DONE,
    TASK_DROP
} task_state_types;

struct task_data {
    float2              pos;
    float2              size;

    int                 seed;
    int                 mode;
    int                 bvhBegin;

    unsigned int        trisCount;
};

struct triangle_data {
    float4              v0;
    float4              v1;
    float4              v2;


    float4              n0;
    float4              n1;
    float4              n2;

    float2              u0;
    float2              u1;
    float2              u2;

    unsigned int        material;

    unsigned int        reserved;
};

struct bvh_data {
    float4              min;
    float4              max;

    float4              reserved;

    unsigned int        offset;
    unsigned int        count;

    int                 left;
    int                 escape;
};

struct material_data {
    float4              diffuse;
    float4              emissive;
};

int rayTrace(const struct ARay *ray, __global struct task_data *task, __global struct triangle_data *triangles, __global struct bvh_data *bvh, float3 *p, float3 *n) {
    int result  = -1;
    float dist  = 99999.9; // Camera far plane

    int current     = task->bvhBegin;
    unsigned int t  = 0;
    bool found      = false;
    while(current != -1) {
        bool intersect  = intersect_aabb(ray, bvh[current].min.xyz, bvh[current].max.xyz, 0);
        if(intersect) {
            for(unsigned int i = bvh[current].offset; i < bvh[current].offset + bvh[current].count; i++) {
                float3 pos;
                if(intersect_triangle(ray, triangles[i].v0.xyz, triangles[i].v1.xyz, triangles[i].v2.xyz, &pos, false)) {
                    float3 d    = pos - ray->pos;
                    float l     = dot(d, d);
                    if(l < dist) {
                        dist    = l;
                        if(p) {
                           *p   = pos;
                        }
                        t       = i;
                        found   = true;
                    }
                }
            }
        }

        if(bvh[current].left != -1 && intersect) {
            current = bvh[current].left;
        } else {
            current = bvh[current].escape;
        }
    }

    if(found) {
        result      = triangles[t].material;
        if(n) {
            float3 d    = triangleWeights(p, triangles[t].v0.xyz, triangles[t].v1.xyz, triangles[t].v2.xyz);
           *n       = normalize(triangles[t].n0.xyz * d.x + triangles[t].n1.xyz * d.y + triangles[t].n2.xyz * d.z);
        }
    }

    return result;
}

float3 collectPhotons(int index, __global struct material_data *materials, const float3 pos, const float3 n) {
    return (float3)(1.0);
}

float3 backTrace(int index, __global struct material_data *materials, const float3 pos, const float3 n) {
    float a     = 0.1;
/*
    float3 light= (float3)(0.0, 1.2,-3.75);

    float3 d    = normalize(pos - light);

    struct ARay ray;
    ray.pos = light;
    ray.dir = d;
    if(rayTrace(&ray, 0, 0) == index) {
        float3 l    = normalize(light - pos);
        a           = min((float)1.0, max(dot(n, l), a));
    }

    float3 rgb      = (float3)(a);

    int m           = pPrimitives[index].params.z;
    return min(pMaterials[m].diffuse.xyz, rgb);
*/
    return (float3)a;
}

struct stack_data {
    struct ARay     ray;
    int             diff;
    int             refl;
    int             refr;
    float           power;
};

void push_front(struct stack_data *stack, struct stack_data *data, int *iterator) {
   stack[*iterator] = *data;
   *iterator = *iterator + 1;
}

struct stack_data pop_front(struct stack_data *stack, int *iterator) {
    struct stack_data result;

    *iterator = *iterator - 1;
    if(*iterator >= 0) {
        result  = stack[*iterator];
    }

    return result;
}

float3 calcRadiance(struct ARay *ray, __global struct task_data *task, __global struct triangle_data *triangles, __global struct bvh_data *bvh, __global struct material_data *materials, uint2 *seed) {
    float3 res  = (float3)(1.0);

    int iterator= 0;
    struct stack_data stack[256];

    struct stack_data data;
    data.ray    = *ray;
    data.diff   = 0;
    data.refl   = 0;
    data.refr   = 0;
    data.power  = 1.0;
    push_front(stack, &data, &iterator);

    while(iterator > 0) {
        float3 pos;
        float3 n;
        data    = pop_front(stack, &iterator);

        if(data.power < 0.001 || data.diff >= 5 || data.refl >= 10 || data.refr >= 10) {
            res = (float3)(0.0);
            continue;
        }

        int index       = rayTrace(&data.ray, task, triangles, bvh, &pos, &n);
        if(index > -1) {
            res = n;
/*
            struct material_data mat    = materials[index];
            bool inside = false;
            if(dot(data.ray.dir, n) > 0) {
                n       = -n;
                inside  = true;
            }

            float ior   = 2.8;

            float c     = 1.0;
            if(mat.emissive.w > 0.0) {
                c       = fresnel(dot(data.ray.pos - pos, n), 1.0 / ior);
            }
            uint8_t mode  = (data.diff == 0) ? (task->mode >> 4) & 0x0F : (task->mode) & 0x0F;
            switch(mode) {
                case BACK_TRACING: {
                    return res * backTrace(index, materials, primitives, pos, n);
                } break;

                case PHOTON_MAPPING: {
                    //return res * collectPhotons(index, materials, primitives, pos, n);
                } break;

                case PATH_TRACING: {
                    float4 r    = rand(seed);
                    // Add next bounce to stack
                    if(r.x < mat.emissive.w * (1.0 - c)) { // REFRACT
                        struct stack_data d;
                        d.ray       = refract(&data.ray, n, pos, (inside) ? ior : 1.0, (inside) ? 1.0 : ior);
                        d.diff      = data.diff;
                        d.refl      = data.refl;
                        d.refr      = data.refr + 1;
                        d.power     = data.power * mat.emissive.w * (1.0 - c);
                        push_front(stack, &d, &iterator);
                    } else {
                        if(r.y < mat.diffuse.w * c) { // REFLECT
                            struct stack_data d;
                            d.ray       = reflect(&data.ray, n, pos);
                            d.diff      = data.diff;
                            d.refl      = data.refl + 1;
                            d.refr      = data.refr;
                            d.power     = data.power * mat.diffuse.w * c;
                            push_front(stack, &d, &iterator);
                        } else { // DIFFUSE
                            // Calc radiance
                            res         = mat.emissive.xyz + res * mat.diffuse.xyz;

                            if(mat.emissive.x > 0.0f || mat.emissive.y > 0.0f || mat.emissive.z > 0.0f) // We don't need to create a new bounce
                                continue;

                            struct stack_data d;
                            d.ray       = diffuse(n, pos, seed);
                            d.diff      = data.diff + 1;
                            d.refl      = data.refl;
                            d.refr      = data.refr;
                            d.power     = data.power;
                            push_front(stack, &d, &iterator);
                        }
                    }
                } break;
                default: break;
            }
*/
        } else {
            res = (float3)(0.0f);
        }
    }

    return res;
}

__kernel void doWork(__global struct task_data *task, __global struct camera_data *camera, __global struct triangle_data *triangles, __global struct bvh_data *bvh, __global struct material_data *materials, __global float4 *out) {
    unsigned int x  = get_global_id(0);
    unsigned int y  = get_global_id(1);

    unsigned int w  = get_global_size(0);
    unsigned int h  = get_global_size(1);

    uint2 state     = (uint2)( task->seed * 2312.72457f + x * 321,
                               task->seed * 7263.26234f + y * 819);

    float u     = task->pos.x + x;
    float v     = task->pos.y + y;
    float pu    = u / task->size.x;
    float pv    = v / task->size.y;

    struct ARay ray = castRay(pu, pv, camera);

    int pos     = x + y * w;
    out[pos]   += (float4)(calcRadiance(&ray, task, triangles, bvh, materials, &state), 1.0f);
}
