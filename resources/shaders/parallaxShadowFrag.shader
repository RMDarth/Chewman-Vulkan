{
    "name": "parallaxShadowFragmentShader",
    "filename": "glsl/parallaxAndShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "normalTex",
        "depthTex",
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