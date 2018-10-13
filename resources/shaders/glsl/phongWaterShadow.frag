#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D reflectionSampler;
layout(set = 1, binding = 1) uniform sampler2D refractionSampler;
layout(set = 1, binding = 2) uniform sampler2D dudvSampler;
layout(set = 1, binding = 3) uniform sampler2D shadowSampler;
layout(set = 1, binding = 4) uniform UBO
{
    vec4 cameraPos;
    DirLight dirLight;
    MaterialInfo materialInfo;
    float time;
    float padding[3];
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec4 fragClipPosition;
layout(location = 5) in vec4 fragLightSpacePos;

layout(location = 0) out vec4 outColor;

const float waveStrength = 0.02;
const float waterTransparency = 0.6;

float computeShadowFactor(vec4 lightSpacePos, vec2 distortion, float shadowStrength)
{
    return 1.0;
    // TODO: rethink shadows on water surface


   // Convert light space position to NDC (normalized device coordinates)
   vec3 lightSpaceReal = lightSpacePos.xyz /= lightSpacePos.w;

   // If the fragment is outside the light's projection then it is outside
   // the light's influence, which means it is in the shadow (notice that
   // such sample would be outside the shadow map image)
   /*if (abs(lightSpaceReal.x) > 1.0 ||
       abs(lightSpaceReal.y) > 1.0 ||
       abs(lightSpaceReal.z) > 1.0)
      return shadowStrength;*/

   // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
   vec2 shadowMapCoord = lightSpaceReal.xy * 0.5 + 0.5;

   // Check if the sample is in the light or in the shadow
    if (lightSpaceReal.z > texture(shadowSampler, shadowMapCoord.xy + distortion).x)
         return shadowStrength; // In the shadow

   // In the light
   return 1.0;
}

void main() {
     vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);

    vec3 lightEffect = CalcDirLight(ubo.dirLight, fragNormal, viewDir, ubo.materialInfo);

    vec3 ndc = (fragClipPosition.xyz / fragClipPosition.w) * 0.5 + 0.5;
    vec2 reflectionCoord = vec2(ndc.x, 1 - ndc.y);
    vec2 refractionCoord = vec2(ndc.x, ndc.y);

    float timeMult = ubo.time * 0.05;
    float clampTime = timeMult - floor(timeMult);
    vec2 dudvCoords =  fragTexCoord * 4;
    vec2 distortion1 = (texture(dudvSampler, vec2(dudvCoords.x + clampTime, dudvCoords.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(dudvSampler, vec2(-dudvCoords.x + clampTime, dudvCoords.y + clampTime)).rg * 2.0 - 1.0) * waveStrength;
    vec2 totalDistortion = distortion1 + distortion2;

    reflectionCoord = reflectionCoord + totalDistortion;
    refractionCoord = refractionCoord + totalDistortion;
    //vec3 result = (ambient + diffuse + specular) * fragColor * texture(texSampler, fragTexCoord).rgb;
    vec3 reflectionResult = mix(texture(reflectionSampler, reflectionCoord).rgb, vec3(0.0, 0.3, 0.5), 0.4);
    vec3 refractionResult = texture(refractionSampler, refractionCoord).rgb;
    float fresnelKoef = dot(viewDir, norm);
    float shadowStrength = 0.95f; //waterTransparency;
    vec3 result =  mix(reflectionResult, refractionResult, fresnelKoef);
    outColor = vec4(mix(result, vec3(0.0, 0.3, 0.5), 1 - waterTransparency) * computeShadowFactor(fragLightSpacePos, totalDistortion * 0.1, shadowStrength), 1.0);
}