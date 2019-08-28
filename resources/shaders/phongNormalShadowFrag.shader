{
    "name": "phongNormalShadowFragmentShader",
    "filename": "glsl/phongNormalAndShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "normalTex",
        "directShadowTex"
    ],
    "uniformList": [
        { "uniformType": "CameraPosition" },
        { "uniformType": "InverseModelMatrix" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "LightSpot" },
        { "uniformType": "LightLine" },
        { "uniformType": "LightPointSimple" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" }
    ]
}