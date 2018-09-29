{
    "name": "phongWaterShadowFragmentShader",
    "filename": "glsl/phongWaterShadow.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "reflectionSampler",
        "refractionSampler",
        "dudvSampler",
        "shadowSampler"
    ],
    "uniformList": [
        { "uniformType": "LightPosition" },
        { "uniformType": "LightColor" },
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightAmbient" },
        { "uniformType": "LightDiffuse" },
        { "uniformType": "LightSpecular" },
        { "uniformType": "LightShininess" },
        { "uniformType": "Time" }
    ]
}