{
    "name": "parallaxShadowVertexShader",
    "filename": "glsl/parallaxAndShadowMap.vert.spv",
    "shaderType": "VertexShader",
    "vertexInfo": {
        "vertexDataFlags": [
            "Position",
            "Color",
            "TexCoord",
            "Normal",
            "Binormal",
            "Tangent"
        ]
    },
    "uniformList": [
        { "uniformType": "ModelMatrix" },
        { "uniformType": "ViewMatrix" },
        { "uniformType": "ProjectionMatrix" },
        { "uniformType": "LightPointViewProjectionList" },
        { "uniformType": "LightDirectViewProjectionList" },
        { "uniformType": "ClipPlane" }
    ]
}