{
    "name": "phongShadowFragmentShader",
    "filename": "glsl/phongShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "directShadowTex",
        "pointShadowTex"
    ],
    "uniformList": [
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "LightSpot" },
        { "uniformType": "LightPoint" },
        { "uniformType": "LightLine" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" }
    ]
}