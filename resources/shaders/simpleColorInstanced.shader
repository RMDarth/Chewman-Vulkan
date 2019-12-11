{
    "name": "simpleColorInstancedVertexShader",
    "filename": "glsl/simpleColorInstanced.vert.spv",
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
        { "uniformType": "ProjectionMatrix" }
    ],
    "bufferList": [
        "ModelMatrixList"
    ]
}