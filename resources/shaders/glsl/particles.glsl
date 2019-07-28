// Particles structures and functions

struct ParticleEmitter
{
    // shape
    mat4 toDirection;
    float angle;
    float originRadius;

    // params
    float emissionRate;
    float minLife;
    float maxLife;
    float minSpeed;
    float maxSpeed;
    float minSize;
    float maxSize;
    float minRotate;
    float maxRotate;

    // changers
    vec4 colorRangeStart;
    vec4 colorRangeEnd;
};

struct ParticleAffector
{
    float minAcceleration;
    float maxAcceleration;
    float minRotateSpeed;
    float maxRotateSpeed;
    float minScaleSpeed;
    float maxScaleSpeed;
    float lifeDrain;
    vec4 colorChanger;
};