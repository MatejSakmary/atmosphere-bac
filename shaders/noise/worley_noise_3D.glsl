/* Taken from here:
    https://github.com/SebLague/Clouds/blob/master/Assets/Scripts/Clouds/Noise/Compute/NoiseGenCompute.compute*/
#version 450

#extension GL_EXT_scalar_block_layout : require
struct Vector{
    float x, y, z;
};

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout (scalar, set = 0, binding = 0) buffer pointsABuffer { Vector pointsA []; };
layout (scalar, set = 0, binding = 1) buffer pointsBBuffer { Vector pointsB []; };
layout (scalar, set = 0, binding = 2) buffer pointsCBuffer { Vector pointsC []; };
layout (std430, set = 0, binding = 3) buffer minMaxBuffer { int minVal; int maxVal; };
layout (std140, set = 0, binding = 4) uniform worleyParamsBuffer
{
    ivec3 numDivisions;
    ivec3 texDimensions;
    vec4 targetChannels;
    float persistence;
};
layout (set = 0, binding = 5, r16f) uniform image3D resultNoise;

const float minMaxPrecision = 10000000.0;

const ivec3 offsets[] =
{
    // centre
    ivec3(0,0,0),
    // front face
    ivec3(0,0,1),
    ivec3(-1,1,1),
    ivec3(-1,0,1),
    ivec3(-1,-1,1),
    ivec3(0,1,1),
    ivec3(0,-1,1),
    ivec3(1,1,1),
    ivec3(1,0,1),
    ivec3(1,-1,1),
    // back face
    ivec3(0,0,-1),
    ivec3(-1,1,-1),
    ivec3(-1,0,-1),
    ivec3(-1,-1,-1),
    ivec3(0,1,-1),
    ivec3(0,-1,-1),
    ivec3(1,1,-1),
    ivec3(1,0,-1),
    ivec3(1,-1,-1),
    // ring around centre
    ivec3(-1,1,0),
    ivec3(-1,0,0),
    ivec3(-1,-1,0),
    ivec3(0,1,0),
    ivec3(0,-1,0),
    ivec3(1,1,0),
    ivec3(1,0,0),
    ivec3(1,-1,0)
};

int maxComponent(ivec3 vec) {
    return max(vec.x, max(vec.y, vec.z));
}

int minComponent(ivec3 vec) {
    return min(vec.x, min(vec.y, vec.z));
}

vec3 getPointOffset(uint bufferToOperateOn, int index)
{
    switch (bufferToOperateOn) 
    {
        case 0:
            return vec3(pointsA[index].x, pointsA[index].y, pointsA[index].z);
        case 1:
            return vec3(pointsB[index].x, pointsB[index].y, pointsB[index].z);
        case 2:
            return vec3(pointsC[index].x, pointsC[index].y, pointsC[index].z);
    }
}
/* Since it's possible (as far as I know at least) to pass variable length arrays or 
   Buffers to function the bufferToOperateOn is needed -> 0 - pointsABuffer ... */
float worley(uint bufferToOperateOn, int numCells, vec3 position)
{
    float minSqrDist = 1.0;
    ivec3 cellID = ivec3(
        floor(position.x * numCells), 
        floor(position.y * numCells),
        floor(position.z * numCells)
    );

    for (int cellOffsetIndex = 0; cellOffsetIndex < 27; cellOffsetIndex++)
    {
        ivec3 adjCellID = cellID + offsets[cellOffsetIndex];
        if(minComponent(adjCellID) == -1 || maxComponent(adjCellID) == numCells)
        {
            /* (adjCellID + numCells) % numCells*/
            ivec3 wrappedID = ivec3(mod(ivec3(adjCellID) + ivec3(numCells), vec3(numCells)));
            int adjCellIndex = wrappedID.x + numCells * (wrappedID.y + wrappedID.z * numCells);
            vec3 wrappedPoint = getPointOffset(bufferToOperateOn, adjCellIndex);
            for (int wrapOffsetIndex = 0; wrapOffsetIndex < 27; wrapOffsetIndex++)
            {
                vec3 sampleOffset = (position - (wrappedPoint + offsets[wrapOffsetIndex]));
                minSqrDist = min(minSqrDist, dot(sampleOffset, sampleOffset));
            }
        } 
        else 
        {
            int adjCellIndex = adjCellID.x + numCells * (adjCellID.y + adjCellID.z * numCells);
            vec3 sampleOffset = position - getPointOffset(bufferToOperateOn, adjCellIndex);
            minSqrDist = min(minSqrDist, dot(sampleOffset, sampleOffset));
        }
    }
    return sqrt(minSqrDist);
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Hash by David_Hoskins
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uvec2(UI0, UI1)
#define UI3 uvec3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

vec3 hash33(vec3 p)
{
	uvec3 q = uvec3(ivec3(p)) * UI3;
	q = (q.x ^ q.y ^ q.z)*UI3;
	return -1. + 2. * vec3(q) * UIF;
}

// Gradient noise by iq (modified to be tileable)
float gradientNoise(vec3 x, float freq)
{
    // grid
    vec3 p = floor(x);
    vec3 w = fract(x);
    
    // quintic interpolant
    vec3 u = w * w * w * (w * (w * 6. - 15.) + 10.);

    
    // gradients
    vec3 ga = hash33(mod(p + vec3(0., 0., 0.), freq));
    vec3 gb = hash33(mod(p + vec3(1., 0., 0.), freq));
    vec3 gc = hash33(mod(p + vec3(0., 1., 0.), freq));
    vec3 gd = hash33(mod(p + vec3(1., 1., 0.), freq));
    vec3 ge = hash33(mod(p + vec3(0., 0., 1.), freq));
    vec3 gf = hash33(mod(p + vec3(1., 0., 1.), freq));
    vec3 gg = hash33(mod(p + vec3(0., 1., 1.), freq));
    vec3 gh = hash33(mod(p + vec3(1., 1., 1.), freq));
    
    // projections
    float va = dot(ga, w - vec3(0., 0., 0.));
    float vb = dot(gb, w - vec3(1., 0., 0.));
    float vc = dot(gc, w - vec3(0., 1., 0.));
    float vd = dot(gd, w - vec3(1., 1., 0.));
    float ve = dot(ge, w - vec3(0., 0., 1.));
    float vf = dot(gf, w - vec3(1., 0., 1.));
    float vg = dot(gg, w - vec3(0., 1., 1.));
    float vh = dot(gh, w - vec3(1., 1., 1.));
	
    // interpolation
    return va + 
           u.x * (vb - va) + 
           u.y * (vc - va) + 
           u.z * (ve - va) + 
           u.x * u.y * (va - vb - vc + vd) + 
           u.y * u.z * (va - vc - ve + vg) + 
           u.z * u.x * (va - vb - ve + vf) + 
           u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

// Fbm for Perlin noise based on iq's blog
float perlinfbm(vec3 p, float freq, int octaves)
{
    float G = exp2(-.85);
    float amp = 1.;
    float noise = 0.;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * gradientNoise(p * freq, freq);
        freq *= 2.;
        amp *= G;
    }
    
    return noise;
}

void main()
{
    vec3 pixPos = vec3(gl_GlobalInvocationID.xyz) / vec3(texDimensions);
    float layerA = worley(0, numDivisions.x, pixPos);
    float layerB = worley(1, numDivisions.y, pixPos);
    float layerC = worley(2, numDivisions.z, pixPos);

    float noiseSum = layerA + (layerB * persistence) + (layerC * persistence * persistence);
    float localMaxVal = 1 + (persistence) + (persistence * persistence);
    noiseSum /= localMaxVal;
    noiseSum = 1 - noiseSum;
    
    if(numDivisions.x == 3.0)
    {
        float pfbm = mix(1.0, perlinfbm(pixPos, 4, 7), 0.5);
        pfbm = abs(pfbm * 2.0 - 1.0);
        pfbm = remap(pfbm, 0.0, 1.0, noiseSum, 1.0);
        noiseSum = pfbm;
    }

    int valueAsInt = int(noiseSum * minMaxPrecision);
    atomicMin(minVal, valueAsInt);
    atomicMax(maxVal, valueAsInt);

    imageStore(resultNoise, ivec3(gl_GlobalInvocationID.xyz), vec4(noiseSum));
}