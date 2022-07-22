#version 450
layout (local_size_x = 8, local_size_y = 4) in;

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (set = 2, binding = 0, rgba16f) uniform image2D transmittanceLUT;
/* ================================== NOT USED ==================================== */
layout (set = 2, binding = 1, rgba16f) uniform readonly image2D multiscatteringLUT;
layout (set = 2, binding = 2, rgba16f) uniform readonly image2D skyViewLUT;
layout (set = 2, binding = 3, rgba16f) uniform readonly image3D AEPerspective;
/* ================================================================================ */

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

vec3 IntegrateTransmittance(vec3 worldPosition, vec3 worldDirection, uint sampleCount)
{
    vec3 planet0 = vec3(0.0, 0.0, 0.0);
    /* The length of ray between position and nearest atmosphere top boundary */
    float integrationLength = raySphereIntersectNearest(
        worldPosition, worldDirection, planet0, atmosphereParameters.top_radius);
    float integrationStep = integrationLength / float(sampleCount);
    /* Result of the integration */
    vec3 opticalDepth = vec3(0.0,0.0,0.0);

    for(int i = 0; i < sampleCount; i++)
    {
        /* Move along the world direction ray to new position */
        vec3 newPos = worldPosition + i * integrationStep * worldDirection;
        vec3 atmosphereExtinction = SampleMediumExtinction(newPos);
        opticalDepth += atmosphereExtinction * integrationStep;
    }
    return opticalDepth;
}

void main()
{
    vec2 atmosphereBoundaries = vec2(
        atmosphereParameters.bottom_radius,
        atmosphereParameters.top_radius);

    vec2 uv = gl_GlobalInvocationID.xy / atmosphereParameters.TransmittanceTexDimensions;
    vec2 LUTParams = UvToTransmittanceLUTParams(uv, atmosphereBoundaries);

    /* Ray origin in World Coordinates */
    vec3 worldPosition = vec3(0.0, 0.0, LUTParams.x);
    /* Ray direction in World Coordinates */
    vec3 worldDirection = vec3(0.0, safeSqrt(1.0 - LUTParams.y * LUTParams.y), LUTParams.y); 
    vec3 transmittance = exp(-IntegrateTransmittance(worldPosition, worldDirection, 400));
    imageStore(transmittanceLUT, ivec2(gl_GlobalInvocationID.xy), vec4(transmittance, 1.0));
}