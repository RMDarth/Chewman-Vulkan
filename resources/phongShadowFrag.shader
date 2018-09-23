{
    "name": "phongShadowFragmentShader",
    "filename": "shaders/phongShadowMap.frag.spv",
    "shaderType": "FragmentShader",
    "samplerNamesList": [
        "diffuseTex",
        "shadowTex"
    ],
    "uniformList": [
        { "uniformType": "LightPosition" },
        { "uniformType": "LightColor" },
        { "uniformType": "CameraPosition" },
        { "uniformType": "LightAmbient" },
        { "uniformType": "LightDiffuse" },
        { "uniformType": "LightSpecular" },
        { "uniformType": "LightShininess" }
    ]
}