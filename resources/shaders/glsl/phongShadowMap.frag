#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D shadowTex;
layout(set = 1, binding = 2) uniform UBO
{
    mat4 lightViewProjection;
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
layout(location = 4) in vec4 fragLightSpacePos;

layout(location = 0) out vec4 outColor;


float computeShadowFactor(vec4 lightSpacePos)
{
   // Convert light space position to NDC (normalized device coordinates)
   //vec3 lightSpaceReal = lightSpacePos.xyz /= lightSpacePos.w;

   // If the fragment is outside the light's projection then it is outside
   // the light's influence, which means it is in the shadow (notice that
   // such sample would be outside the shadow map image)
   // TODO: Use something like "isFiniteShadow" uniform for this check
   /*if (abs(lightSpaceReal.x) > 1.0 ||
       abs(lightSpaceReal.y) > 1.0 ||
       abs(lightSpaceReal.z) > 1.0)
      return 0.4;*/


   // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
   vec2 shadowMapCoord = lightSpacePos.xy * 0.5 + 0.5;

   // Check if the sample is in the light or in the shadow
    if (lightSpacePos.z > texture(shadowTex, shadowMapCoord.xy).x)
    {
         return 1 - (lightSpacePos.w - 0.4); // In the shadow
    }

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


    vec4 shadowTexCoord = ubo.lightViewProjection * vec4(fragPos, 1);// * 0.5 + 0.5;
    //vec3 result = (ambient + diffuse + specular) * fragColor * texture(diffuseTex, fragTexCoord).rgb * computeShadowFactor(fragLightSpacePos);
    vec3 result = vec3(texture(diffuseTex, fragTexCoord).rgb) * computeShadowFactor(shadowTexCoord);
   // vec3 result = vec3(fragNormal);
    outColor = vec4(result, 1.0);
}