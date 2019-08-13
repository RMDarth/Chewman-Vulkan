{
    "name": "phongNormalShadowFragmentShader",
    "filename": "glsl/phongNormalAndShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "normalTex",
        "directShadowTex",
        "pointShadowTex"
    ],
    "uniformList": [
        { "uniformType": "CameraPosition" },
        { "uniformType": "InverseModelMatrix" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "LightSpot" },
        { "uniformType": "LightPoint" },
        { "uniformType": "LightLine" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" }
    ]
}