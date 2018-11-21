#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    float alpha = 1.0 - dot(fragTexCoord, fragTexCoord);
    if(0.2 > alpha)
    {
        discard;
    }
    outColor = vec4(fragColor.rgb, alpha);
}
