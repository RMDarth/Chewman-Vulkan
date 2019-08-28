{
    "name": "phongShadowEmitFragmentShader",
    "filename": "glsl/phongShadowMapEmit.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "emitTex",
        "directShadowTex"
    ],
    "uniformList": [
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightDirectional" },
        { "uniformType": "LightSpot" },
        { "uniformType": "LightPoint" },
        { "uniformType": "LightLine" },
        { "uniformType": "LightPointSimple" },
        { "uniformType": "LightInfo" },
        { "uniformType": "MaterialInfo" }
    ]
}