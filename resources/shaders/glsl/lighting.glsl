// Copyright (c) 2018-2019, Igor Barinov
// Some lighting functions based on code from learnopengl.com

struct DirLight
{
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct PointLight
{
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constAttenuation;
    float linear;
    float quadratic;
    float padding;
};

struct PlaneLight
{
    vec4 position1;
    vec4 position2;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constAttenuation;
    float linear;
    float quadratic;
    float padding;
};

struct SpotLight
{
    vec4 position;
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float cutOff;
    float outerCutOff;

    float constAttenuation;
    float linear;
    float quadratic;
    // float[3] padding
};

struct LineLight
{
    vec4 startPosition;
    vec4 endPosition;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constAttenuation;
    float linear;
    float quadratic;
    float padding;
};

const uint LI_DirectionalLight =   1 << 0;
const uint LI_PointLight[4] =     {1 << 1,
                                   1 << 2,
                                   1 << 3,
                                   1 << 4};
const uint LI_SpotLight =          1 << 5;

struct LightInfo
{
    uint isSimpleLight;
    uint enableShadows;
    uint lightLineNum;
    uint lightPointsNum;
};

struct MaterialInfo
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    uint ignoreShadow;
    // float[2] padding
};

float sqLen(vec3 v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, MaterialInfo material)
{
    vec3 lightDir = normalize(-vec3(light.direction));
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = vec3(light.ambient * material.diffuse);
    vec3 diffuse = vec3(light.diffuse * diff * material.diffuse);
    vec3 specular = vec3(light.specular * spec * material.specular);
    return (ambient + diffuse + specular);
}

vec3 CalcSimplePointLight(PointLight light, vec3 fragPos, MaterialInfo material)
{
    vec3 lightDir = vec3(light.position) - fragPos;
    float dist = sqLen(lightDir);
    if (dist > 16)
        return vec3(0);
    return vec3(smoothstep(16.0, 0.0, dist) * material.diffuse * 0.4);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, MaterialInfo material)
{
    vec3 lightDir = vec3(light.position) - fragPos;
    float dist = sqLen(lightDir);
    if (dist > 5 * 5)
        return vec3(0,0,0);
    float koef = smoothstep(5*5, 0.0, dist);

    lightDir = normalize(lightDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(vec3(light.position) - fragPos);
    float attenuation = 1.0 / (light.constAttenuation + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = vec3(light.ambient * material.diffuse);
    vec3 diffuse = vec3(light.diffuse * diff * material.diffuse);
    vec3 specular = vec3(light.specular * spec * material.specular);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular) * koef;
}

vec3 CalcSimpleLineLight(LineLight light, vec3 normal, vec3 fragPos, vec3 viewDir, MaterialInfo material)
{
    vec3 AP = fragPos - vec3(light.startPosition);
    vec3 AB = light.endPosition.xyz - light.startPosition.xyz;

    float ABlenSquare = AB.x*AB.x + AB.y*AB.y + AB.z*AB.z;
    float distance = dot(AP, AB) / ABlenSquare;

    //distance = clamp(distance, 0.0, 1.0);
    vec3 lightPos = light.startPosition.xyz + AB * distance;

    // get plane normal
    vec3 planeNormal = normalize(cross(AP, AB));
    float normalProj = dot(normal, planeNormal);
    vec3 finalNormalPos = (fragPos + normal) - planeNormal * normalProj;
    vec3 finalNormal = normalize(finalNormalPos - fragPos);

    vec3 PXNorm = normalize(lightPos - fragPos);
    float PX = length(lightPos - fragPos);
    vec3 GoodDir = normalize(finalNormal + PXNorm);
    float cosA = dot(PXNorm, GoodDir);
    float PY = PX / cosA;

    vec3 Y = fragPos + GoodDir * PY;
    vec3 AY = Y - light.startPosition.xyz;
    float AYproj = dot(AY, AB);
    vec3 finalPoint = Y;
    if (AYproj < 0)
    finalPoint = light.startPosition.xyz;
    if (AYproj > ABlenSquare)
    finalPoint = light.endPosition.xyz;

    PointLight pl;
    pl.position = vec4(finalPoint, 1);
    pl.ambient = light.ambient;
    pl.diffuse = light.diffuse;
    pl.specular = light.specular;
    pl.constAttenuation = light.constAttenuation;
    pl.linear = light.linear;
    pl.quadratic = light.quadratic;

    return CalcSimplePointLight(pl, fragPos, material);
}

vec3 CalcLineLight(LineLight light, vec3 normal, vec3 fragPos, vec3 viewDir, MaterialInfo material)
{
    vec3 AP = fragPos - vec3(light.startPosition);
    vec3 AB = light.endPosition.xyz - light.startPosition.xyz;

    float ABlenSquare = AB.x*AB.x + AB.y*AB.y + AB.z*AB.z;
    float distance = dot(AP, AB) / ABlenSquare;

    //distance = clamp(distance, 0.0, 1.0);
    vec3 lightPos = light.startPosition.xyz + AB * distance;

    // get plane normal
    vec3 planeNormal = normalize(cross(AP, AB));
    float normalProj = dot(normal, planeNormal);
    vec3 finalNormalPos = (fragPos + normal) - planeNormal * normalProj;
    vec3 finalNormal = normalize(finalNormalPos - fragPos);

    vec3 PXNorm = normalize(lightPos - fragPos);
    float PX = length(lightPos - fragPos);
    vec3 GoodDir = normalize(finalNormal + PXNorm);
    float cosA = dot(PXNorm, GoodDir);
    float PY = PX / cosA;

    vec3 Y = fragPos + GoodDir * PY;
    vec3 AY = Y - light.startPosition.xyz;
    float AYproj = dot(AY, AB);
    vec3 finalPoint = Y;
    if (AYproj < 0)
        finalPoint = light.startPosition.xyz;
    if (AYproj > ABlenSquare)
        finalPoint = light.endPosition.xyz;

    PointLight pl;
    pl.position = vec4(finalPoint, 1);
    pl.ambient = light.ambient;
    pl.diffuse = light.diffuse;
    pl.specular = light.specular;
    pl.constAttenuation = light.constAttenuation;
    pl.linear = light.linear;
    pl.quadratic = light.quadratic;

    return CalcPointLight(pl, normal, fragPos, viewDir, material);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, MaterialInfo material)
{
    vec3 lightPos = vec3(light.position);
    vec3 lightDir = normalize(lightPos - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (light.constAttenuation + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-vec3(light.direction)));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = vec3(light.ambient * material.diffuse);
    vec3 diffuse = vec3(light.diffuse * diff * material.diffuse);
    vec3 specular = vec3(light.specular * spec * material.specular);
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
