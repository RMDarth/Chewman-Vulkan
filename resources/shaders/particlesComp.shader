{
    "name": "particlesComputeShader",
    "filename": "glsl/particles.comp.spv",
    "shaderType": "ComputeShader",
    "vertexInfo": {
        "vertexDataFlags": []
    },
    "uniformList": [
        { "uniformType": "ParticleEmitter" },
        { "uniformType": "ParticleAffector" },
        { "uniformType": "ParticleCount" },
        { "uniformType": "Time" },
        { "uniformType": "DeltaTime" }
    ],
    "bufferList": [
        "AtomicCounter"
    ]

}