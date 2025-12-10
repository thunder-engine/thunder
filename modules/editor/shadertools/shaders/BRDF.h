float F_Schlick(float VdotH, float f0) {
    float Fc = exp2((-5.55473 * VdotH - 6.98316) * VdotH);
    return f0 + (clamp(50.0 * f0, 0.0, 1.0) - f0) * Fc;
}

float F_Fresnel(float VdotH, float f0) {
    float s = sqrt(clamp(0.0, 0.999, f0));
    float n = (1.0 + s) / (1.0 - s);
    float g = sqrt(sqr(n) + sqr(VdotH) - 1.0);
    return 0.5 * sqr((g - VdotH) / (g + VdotH)) * (1.0 + sqr(((g + VdotH) * VdotH - 1.0) / ((g - VdotH) * VdotH + 1.0)));
}

// [Blinn 1977, "Models of light reflection for computer synthesized pictures"]
float D_Blinn(float NdotH, float r) {
    float M = sqr(r);
    float M2 = sqr(M);

    float n = 2.0 / M2 - 2.0;
    return(n + 2.0) / (2.0 * PI) * pow(NdotH, n);
}

// [Beckmann 1963, "The scattering of electromagnetic waves from rough surfaces"]
float D_Beckmann(float NdotH, float r) {
    float M = sqr(r);
    float M2 = sqr(M);

    float T2 = sqr(NdotH);
    //exp((T2 - 1) / (M * T2)) / (M * T2 * T2);
    return exp((T2 - 1) / (M2 * T2)) / (PI * M2 * T2 * T2);
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float NdotH, float r) {
    float M = sqr(r);
    float M2 = sqr(M);

    return M2 / (PI * sqr((NdotH * M2 - NdotH) * NdotH + 1.0));
    //float T2 = sqr(NdotH);
    //return(1.0 / PI) * sqr(r / (T2 *(M +(1 - T2) / T2)));
}

// [Neumann et al. 1999, "Compact metallic reflectance models"]
float V_Neumann(float NdotV, float NdotL) {
    return 1 / (4 * max(NdotL, NdotV));
}

float V_Schlick(float NdotV, float NdotL, float r) {
    float k = sqr(r) * 0.5;
    float v = NdotV * (1.0 - k) + k;
    float l = NdotL * (1.0 - k) + k;
    return 0.25 / (v * l);
}

// Smith term for GGX
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
float V_Smith(float NdotV, float NdotL, float r) {
    float A = sqr(r);
    float A2 = sqr(A);

    float v = NdotV + sqrt(NdotV *(NdotV - NdotV * A2) + A2);
    float l = NdotL + sqrt(NdotL *(NdotL - NdotL * A2) + A2);
    return 1.0 / (v * l);
}

float getNormalDistributionFunction(float NdotH, float r) {
    return D_GGX(NdotH, r);
}

// V = G / (4 * NdotL * NdotV)
float getGeometricVisibility(float NdotL, float NdotV, float r) {
    return clamp(V_Schlick(NdotV, NdotL, r), 0.0, 1.0);
}

float getFresnel(float VdotH, float s) {
    return F_Schlick(VdotH, s);
}

float getLambert(float NdotL, float b) {
    return max(NdotL * b, 0.0);
}

float getCookTorrance(vec3 n, vec3 v, vec3 h, float NdotL, float r) {
    float NdotV = clamp(dot(n, v), 0.0, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    float G = getGeometricVisibility(NdotL, NdotV, r);
    float F = getFresnel(VdotH, 0.5);
    float D = getNormalDistributionFunction(NdotH, r);

    return G * F * D;
}

vec3 closestPointOnLine(vec3 a, vec3 b, vec3 c) {
    vec3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + t * ab;
}

vec3 closestPointOnSegment(vec3 a, vec3 b, vec3 p) {
    vec3 v = b - a;
    vec3 w = p - a;

    float c1 = dot(v, w);
    float c2 = dot(v, v);
    return a + v * clamp(c1 / c2, 0.0, 1.0);
}
// Shadow map functions
float getShadowSample(sampler2D map, vec2 coord, float t) {
    return step(t, texture(map, coord).x);
}

float getShadowSampleLinear(sampler2D map, vec2 coord, float t) {
    vec2 pos = coord / shadowPageSize() + vec2(0.5f);
    vec2 frac = fract(pos);
    vec2 start = (pos - frac) * shadowPageSize();

    float bl = getShadowSample(map, start, t);
    float br = getShadowSample(map, start + vec2(shadowPageSize(), 0.0f), t);
    float tl = getShadowSample(map, start + vec2(0.0f, shadowPageSize()), t);
    float tr = getShadowSample(map, start + vec2(shadowPageSize()), t);

    float a = mix(bl, tl, frac.y);
    float b = mix(br, tr, frac.y);

    return mix(a, b, frac.x);
}

float getShadowSamplePCF(sampler2D map, vec2 coord, float t) {
    const float NUM_SAMPLES = 4.0f;
    const float SAMPLES_START = (NUM_SAMPLES - 1.0f) * 0.5;

    float result = 0.0f;
    for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f) {
        for(float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f) {
            result += getShadowSampleLinear(map, coord + vec2(x, y) * shadowPageSize(), t);
        }
    }
    return result / (NUM_SAMPLES * NUM_SAMPLES);
}

float getShadowVarianceSample(sampler2D map, vec2 coord, float t) {
    vec2 m = texture(map, coord).xy;

    float p = step(t, m.x);
    float v = max(m.y - m.x * m.x, 0.000002f); // 0.000002 = Min variance

    float d = t - m.x;
    float pm = linstep(0.2, 1.0f, v / (v + d * d));

    return max(p, pm);
}

float getShadow(sampler2D map, vec2 coord, float t) {
    return clamp(getShadowSamplePCF(map, coord, t), 0.0f, 1.0f);
}
