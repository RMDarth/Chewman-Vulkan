// Lighting functions based on code from learnopengl.com

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

    float constant;
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

    float constant;
    float linear;
    float quadratic;
    // float[3] padding
};

const uint LI_DirectionalLight =   1 << 0;
const uint LI_PointLight[4] =     {1 << 1,
                                   1 << 2,
                                   1 << 3,
                                   1 << 4};
const uint LI_SpotLight =          1 << 5;

struct LightInfo
{
    uint lightFlags;
    // float[3] padding
};

struct MaterialInfo
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
    // float[3] padding
};

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

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, MaterialInfo material)
{
    vec3 lightDir = normalize(vec3(light.position) - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(vec3(light.position) - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = vec3(light.ambient * material.diffuse);
    vec3 diffuse = vec3(light.diffuse * diff * material.diffuse);
    vec3 specular = vec3(light.specular * spec * material.specular);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
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
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
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