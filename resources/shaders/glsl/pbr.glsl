#define PI 3.1415926
#define RIM_LIGHT 0.01

#define COOK
#define COOK_GGX
//#define PHONG

// handy value clamping to 0 - 1 range
float saturate(float value)
{
    return clamp(value, 0.0, 1.0);
}


// phong (lambertian) diffuse term
float phong_diffuse()
{
    return (1.0 / PI);
}


// compute fresnel specular factor for given base specular and product
// product could be NdV or VdH depending on used technique
vec3 fresnel_factor(vec3 f0, float product)
{
    return mix(f0, vec3(1.0), pow(1.01 - product, 5.0));
}

float FresnelSchlickWithRoughness(float SpecularColor, vec3 V, vec3 N, float Gloss)
{
    return SpecularColor + (max(Gloss, SpecularColor) - SpecularColor) * clamp(pow(1.0 - dot(V, N), 5.0), 0.0, 1.0);
}

// following functions are copies of UE4
// for computing cook-torrance specular lighting terms
float D_blinn(float roughness, float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float n = 2.0 / m2 - 2.0;
    return (n + 2.0) / (2.0 * PI) * pow(NdH, n);
}

float D_beckmann(float roughness, float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float NdH2 = NdH * NdH;
    return exp((NdH2 - 1.0) / (m2 * NdH2)) / (PI * m2 * NdH2 * NdH2);
}

float D_GGX(float roughness, float NdH)
{
    float m = roughness * roughness;
    float m2 = m * m;
    float d = (NdH * m2 - NdH) * NdH + 1.0;
    return m2 / (PI * d * d);
}

float G_schlick(float roughness, float NdV, float NdL)
{
    float k = roughness * roughness * 0.5;
    float V = NdV * (1.0 - k) + k;
    float L = NdL * (1.0 - k) + k;
    return 0.25 / (V * L);
}


// simple phong specular calculation with normalization
vec3 phong_specular(vec3 V, vec3 L, vec3 N, vec3 specular, float roughness)
{
    vec3 R = reflect(-L, N);
    float spec = max(0.0, dot(V, R));

    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(spec, min(10000.0, k)) * specular;
}

// simple blinn specular calculation with normalization
vec3 blinn_specular(float NdH, vec3 specular, float roughness)
{
    float k = 1.999 / (roughness * roughness);

    return min(1.0, 3.0 * 0.0398 * k) * pow(NdH, min(10000.0, k)) * specular;
}

// cook-torrance specular calculation
#ifdef COOK
vec3 cooktorrance_specular( float NdL, float NdV, float NdH, vec3 specular, float roughness)
{
    #ifdef COOK_BLINN
    float D = D_blinn(roughness, NdH);
    #endif

    #ifdef COOK_BECKMANN
    float D = D_beckmann(roughness, NdH);
    #endif

    #ifdef COOK_GGX
    float D = D_GGX(roughness, NdH);
    #endif

    float G = G_schlick(roughness, NdV, NdL);

    float rim = mix(1.0 - roughness * RIM_LIGHT * 0.9, 1.0, NdV);

    return (1.0 / rim) * specular * G * D;
}
#endif
