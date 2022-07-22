#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 64) in;

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (set = 2, binding = 0, rgba16f) uniform readonly image2D transmittanceLUT;
layout (set = 2, binding = 1, rgba16f) uniform  image2D multiscatteringLUT;
/* ================================== NOT USED ==================================== */
layout (set = 2, binding = 2, rgba16f) uniform readonly image2D skyViewLUT;
layout (set = 2, binding = 3, rgba16f) uniform readonly image3D AEPerspective;
/* ================================================================================ */

/* This number should match the number of local threads -> z dimension */
const float SPHERE_SAMPLES = 64.0;
const float GOLDEN_RATIO = 1.6180339;
const float uniformPhase = 1.0 / (4.0 * PI);

shared vec3 MultiscattSharedMem[64];
shared vec3 LSharedMem[64];

struct RaymarchResult 
{
    vec3 Luminance;
    vec3 Multiscattering;
};


/* ============================= MEDIUM SAMPLING ============================ */
vec3 SampleMediumExtinction(vec3 worldPosition)
{
    const float viewHeight = length(worldPosition) - atmosphereParameters.bottom_radius;

    const float densityMie = exp(atmosphereParameters.mie_density[1].w * viewHeight);
    const float densityRay = exp(atmosphereParameters.rayleigh_density[1].w * viewHeight);
    const float densityOzo = clamp(viewHeight < atmosphereParameters.absorption_density[0].x ?
        atmosphereParameters.absorption_density[0].w * viewHeight + atmosphereParameters.absorption_density[1].x :
        atmosphereParameters.absorption_density[2].x * viewHeight + atmosphereParameters.absorption_density[2].y,
        0.0, 1.0);

    vec3 mieExtinction = atmosphereParameters.mie_extinction * densityMie;
    vec3 rayleighExtinction = atmosphereParameters.rayleigh_scattering * densityRay;
    vec3 ozoneExtinction = atmosphereParameters.absorption_extinction * densityOzo; 
    
    return mieExtinction + rayleighExtinction + ozoneExtinction;
}

vec3 SampleMediumScattering(vec3 worldPosition)
{
    const float viewHeight = length(worldPosition) - atmosphereParameters.bottom_radius;

    const float densityMie = exp(atmosphereParameters.mie_density[1].w * viewHeight);
    const float densityRay = exp(atmosphereParameters.rayleigh_density[1].w * viewHeight);
    const float densityOzo = clamp(viewHeight < atmosphereParameters.absorption_density[0].x ?
        atmosphereParameters.absorption_density[0].w * viewHeight + atmosphereParameters.absorption_density[1].x :
        atmosphereParameters.absorption_density[2].x * viewHeight + atmosphereParameters.absorption_density[2].y,
        0.0, 1.0);

    vec3 mieScattering = atmosphereParameters.mie_scattering * densityMie;
    vec3 rayleighScattering = atmosphereParameters.rayleigh_scattering * densityRay;
    /* Not considering ozon scattering in current version of this model */
    vec3 ozoneScattering = vec3(0.0, 0.0, 0.0);
    
    return mieScattering + rayleighScattering + ozoneScattering;
}
/* ========================================================================== */

RaymarchResult IntegrateScatteredLuminance(vec3 worldPosition, vec3 worldDirection, 
    vec3 sunDirection, float sampleCount)
{
    RaymarchResult result = RaymarchResult(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
    vec3 planet0 = vec3(0.0, 0.0, 0.0);
    float planetIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphereParameters.bottom_radius);
    float atmosphereIntersectionDistance = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphereParameters.top_radius);
    
    float integrationLength;
    /* ============================= CALCULATE INTERSECTIONS ============================ */
    if((planetIntersectionDistance == -1.0) && (atmosphereIntersectionDistance == -1.0)){
        /* ray does not intersect planet or atmosphere -> no point in raymarching*/
        return result;
    } 
    else if((planetIntersectionDistance == -1.0) && (atmosphereIntersectionDistance > 0.0)){
        /* ray intersects only atmosphere */
        integrationLength = atmosphereIntersectionDistance;
    }
    else if((planetIntersectionDistance > 0.0) && (atmosphereIntersectionDistance == -1.0)){
        /* ray intersects only planet */
        integrationLength = planetIntersectionDistance;
    } else {
        /* ray intersects both planet and atmosphere -> return the first intersection */
        integrationLength = min(planetIntersectionDistance, atmosphereIntersectionDistance);
    }
    float integrationStep = integrationLength / float(sampleCount);

    vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);

    /* stores accumulated transmittance during the raymarch process */
    vec3 accumTrans = vec3(1.0, 1.0, 1.0);
    /* stores accumulated light contribution during the raymarch process */
    vec3 accumLight = vec3(0.0, 0.0, 0.0);
    float oldRayShift = 0;

    /* ============================= RAYMARCH ==========================================  */
    for(int i = 0; i < sampleCount; i++)
    {
        /* Sampling at 1/3rd of the integration step gives better results for exponential
           functions */
        float newRayShift = integrationLength * (float(i) + 0.3) / sampleCount;
        integrationStep = newRayShift - oldRayShift;
        vec3 newPos = worldPosition + newRayShift * worldDirection;
        oldRayShift = newRayShift;

        /* Raymarch shifts the angle to the sun a bit recalculate */
        vec3 upVector = normalize(newPos);
        vec2 transLUTParams = vec2( length(newPos),dot(sunDirection, upVector));

        /* uv coordinates later used to sample transmittance texture */
        vec2 transUV = TransmittanceLUTParamsToUv(transLUTParams, atmosphereBoundaries);
        /* because here transmittanceLUT is image and not a texture transfer
           from [0,1] -> [tex_width, tex_height] */
        ivec2 transImageCoords = ivec2(transUV * atmosphereParameters.TransmittanceTexDimensions);

        vec3 transmittanceToSun = vec3(imageLoad(transmittanceLUT, transImageCoords).rgb);
        vec3 mediumScattering = SampleMediumScattering(newPos);
        vec3 mediumExtinction = SampleMediumExtinction(newPos);

        /* TODO: This probably should be a texture lookup*/
        vec3 transIncreseOverInegrationStep = exp(-(mediumExtinction * integrationStep));
        /* Check if current position is in earth's shadow */
        float earthIntersectionDistance = raySphereIntersectNearest(
            newPos, sunDirection, planet0 + PLANET_RADIUS_OFFSET * upVector, atmosphereParameters.bottom_radius);
        float inEarthShadow = earthIntersectionDistance == -1.0 ? 1.0 : 0.0;

        /* Light arriving from the sun to this point */
        vec3 sunLight = inEarthShadow * transmittanceToSun * mediumScattering * uniformPhase;
        vec3 multiscatteredContInt = 
            (mediumScattering - mediumScattering * transIncreseOverInegrationStep) / mediumExtinction;
        vec3 inscatteredContInt = 
            (sunLight - sunLight * transIncreseOverInegrationStep) / mediumExtinction;
        /* For some reson I need to do this to avoid nans in multiscatteredLightInt -> 
           precision error? */
        if(all(equal(transIncreseOverInegrationStep, vec3(1.0)))) { 
            multiscatteredContInt = vec3(0.0); 
            inscatteredContInt = vec3(0.0);
        }
        result.Multiscattering += accumTrans * multiscatteredContInt;
        accumLight += accumTrans * inscatteredContInt;
        // accumLight = accumTrans;
        accumTrans *= transIncreseOverInegrationStep;
    }
    result.Luminance = accumLight;
    return result;
    /* TODO: Check for bounced light off the earth */
}

void main()
{
    const float sampleCount = 20;

    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + vec2(0.5, 0.5)) / atmosphereParameters.MultiscatteringTexDimensions;
    uv = vec2(fromSubUvsToUnit(uv.x, atmosphereParameters.MultiscatteringTexDimensions.x),
        fromSubUvsToUnit(uv.y, atmosphereParameters.MultiscatteringTexDimensions.y));
    
    /* Mapping uv to multiscattering LUT parameters
       TODO -> Is the range from 0.0 to -1.0 really needed? */
    float sunCosZenithAngle = uv.x * 2.0 - 1.0;
    vec3 sunDirection = vec3(
        0.0,
        sqrt(clamp(1.0 - sunCosZenithAngle * sunCosZenithAngle, 0.0, 1.0)),
        sunCosZenithAngle
    );

   float viewHeight = atmosphereParameters.bottom_radius + 
        clamp(uv.y + PLANET_RADIUS_OFFSET, 0.0, 1.0) *
        (atmosphereParameters.top_radius - atmosphereParameters.bottom_radius - PLANET_RADIUS_OFFSET);

    vec3 worldPosition = vec3(0.0, 0.0, viewHeight);

    float sampleIdx = gl_LocalInvocationID.z;
    // local thread dependent raymarch
    { 
        #define USE_HILL_SAMPLING 0
        #if USE_HILL_SAMPLING
            #define SQRTSAMPLECOUNT 8
            const float sqrtSample = float(SQRTSAMPLECOUNT);
            float i = 0.5 + float(sampleIdx / SQRTSAMPLECOUNT);
            float j = 0.5 + mod(sampleIdx, SQRTSAMPLECOUNT);
            float randA = i / sqrtSample;
            float randB = j / sqrtSample;

            float theta = 2.0 * PI * randA;
            float phi = PI * randB;
        #else
        /* Fibbonaci lattice -> http://extremelearning.com.au/how-to-evenly-distribute-points-on-a-sphere-more-effectively-than-the-canonical-fibonacci-lattice/ */
            float theta = acos( 1.0 - 2.0 * (sampleIdx + 0.5) / SPHERE_SAMPLES );
            float phi = (2 * PI * sampleIdx) / GOLDEN_RATIO;
        #endif


        vec3 worldDirection = vec3( cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
        RaymarchResult result = IntegrateScatteredLuminance(worldPosition, worldDirection, 
            sunDirection, sampleCount);

        MultiscattSharedMem[gl_LocalInvocationID.z] = result.Multiscattering / SPHERE_SAMPLES;
        LSharedMem[gl_LocalInvocationID.z] = result.Luminance / SPHERE_SAMPLES;
    }

    groupMemoryBarrier();
    barrier();

    if(gl_LocalInvocationID.z < 32)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 32];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 32];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z < 16)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 16];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 16];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z < 8)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 8];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 8];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z < 4)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 4];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 4];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z < 2)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 2];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 2];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z < 1)
    {
        MultiscattSharedMem[gl_LocalInvocationID.z] += MultiscattSharedMem[gl_LocalInvocationID.z + 1];
        LSharedMem[gl_LocalInvocationID.z] += LSharedMem[gl_LocalInvocationID.z + 1];
    }
    groupMemoryBarrier();
    barrier();
    if(gl_LocalInvocationID.z != 0)
        return;

    vec3 MultiscattSum = MultiscattSharedMem[0];
    vec3 InScattLumSum = LSharedMem[0];

    const vec3 r = MultiscattSum;
    const vec3 SumOfAllMultiScatteringEventsContribution = vec3(1.0/ (1.0 -r.x),1.0/ (1.0 -r.y),1.0/ (1.0 -r.z));
    vec3 Lum = InScattLumSum * SumOfAllMultiScatteringEventsContribution;

    imageStore(multiscatteringLUT, ivec2(gl_GlobalInvocationID.xy), vec4(Lum, 1.0));

}