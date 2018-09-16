#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform UBO
{
    vec4 lightPos;
	vec4 lightColor;
	vec4 cameraPos;
	float ambientStrength;
	float diffuseStrength;
    float specularStrength;
    float shininess;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    // ambient
    vec3 ambient = ubo.ambientStrength * ubo.lightColor.rgb;

    // diffuse
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(ubo.lightPos.xyz - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * ubo.diffuseStrength * ubo.lightColor.rgb;

    // specular
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfVec = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), ubo.shininess);
    vec3 specular = ubo.specularStrength * spec * ubo.lightColor.rgb;

    vec3 result = (ambient + diffuse + specular) * fragColor * texture(texSampler, fragTexCoord).rgb;
    outColor = vec4(result, 1.0);
}