{
    "name": "phongShadowInstancedVertexShader",
    "filename": "glsl/phongShadowMapInstanced.vert.spv",
    "shaderType": "VertexShader",
    "vertexInfo": {
        "vertexDataFlags": [
            "Position",
            "Color",
            "TexCoord",
            "Normal"
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