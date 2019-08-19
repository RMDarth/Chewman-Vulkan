{
    "name": "phongWaterFragmentShader",
    "filename": "glsl/phongWater.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "reflectionSampler",
        "refractionSampler",
        "dudvSampler"
    ],
    "uniformList": [
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "LightSpot" },
        { "uniformType": "LightPoint" },
        { "uniformType": "LightLine" },
        { "uniformType": "LightPointSimple" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" },
        { "uniformType": "Time" }
    ]
}