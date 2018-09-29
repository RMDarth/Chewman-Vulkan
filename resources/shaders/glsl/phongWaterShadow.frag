#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D reflectionSampler;
layout(set = 1, binding = 1) uniform sampler2D dudvSampler;
layout(set = 1, binding = 2) uniform sampler2D shadowSampler;
layout(set = 1, binding = 3) uniform UBO
{
    vec4 lightPos;
	vec4 lightColor;
	vec4 cameraPos;
	float ambientStrength;
	float diffuseStrength;
    float specularStrength;
    float shininess;
    float time;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec4 fragClipPosition;
layout(location = 5) in vec4 fragLightSpacePos;

layout(location = 0) out vec4 outColor;

const float waveStrength = 0.02;

float computeShadowFactor(vec4 lightSpacePos)
{
   // Convert light space position to NDC (normalized device coordinates)
   vec3 lightSpaceReal = lightSpacePos.xyz /= lightSpacePos.w;

   // If the fragment is outside the light's projection then it is outside
   // the light's influence, which means it is in the shadow (notice that
   // such sample would be outside the shadow map image)
   if (abs(lightSpaceReal.x) > 1.0 ||
       abs(lightSpaceReal.y) > 1.0 ||
       abs(lightSpaceReal.z) > 1.0)
      return 0.1;

   // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
   vec2 shadowMapCoord = lightSpaceReal.xy * 0.5 + 0.5;

   // Check if the sample is in the light or in the shadow
    if (lightSpaceReal.z > texture(shadowSampler, shadowMapCoord.xy).x)
         return 0.1; // In the shadow

   // In the light
   return 1.0;
}

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

    vec3 ndc = (fragClipPosition.xyz / fragClipPosition.w) * 0.5 + 0.5;
    vec2 reflectionCoord = vec2(ndc.x, 1 - ndc.y);

    float timeMult = ubo.time * 0.05;
    float clampTime = timeMult - floor(timeMult);
    vec2 dudvCoords =  fragTexCoord * 4;
    vec2 distortion1 = (texture(dudvSampler, vec2(dudvCoords.x + clampTime, dudvCoords.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(dudvSampler, vec2(-dudvCoords.x + clampTime, dudvCoords.y + clampTime)).rg * 2.0 - 1.0) * waveStrength;
    vec2 totalDistortion = distortion1 + distortion2;
    reflectionCoord = reflectionCoord + totalDistortion;
    //vec3 result = (ambient + diffuse + specular) * fragColor * texture(texSampler, fragTexCoord).rgb;
    vec3 result = texture(reflectionSampler, reflectionCoord).rgb * computeShadowFactor(fragLightSpacePos);
    outColor = vec4(mix(result, vec3(0.0, 0.3, 0.5), 0.2), 1.0);
}