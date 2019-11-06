{
    "name": "parallaxShadowInstancedVertexShader",
    "filename": "glsl/parallaxAndShadowMapInstanced.vert.spv",
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
        { "uniformType": "LightDirectViewProjection" },
        { "uniformType": "ClipPlane" }
    ],
    "bufferList": [
        "ModelMatrixList"
    ]
}