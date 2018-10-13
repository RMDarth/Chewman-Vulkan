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
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "MaterialInfo" },
        { "uniformType": "Time" }
    ]
}