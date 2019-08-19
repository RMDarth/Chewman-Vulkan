{
    "name": "parallaxShadowFragmentShader",
    "filename": "glsl/parallaxAndShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "normalTex",
        "depthTex",
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
        { "uniformType": "LightPointSimple" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" }
    ]
}