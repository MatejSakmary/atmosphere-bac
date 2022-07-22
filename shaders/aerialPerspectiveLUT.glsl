#version 450

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"


/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (set = 2, binding = 0, rgba16f) uniform readonly image2D transmittanceLUT;
layout (set = 2, binding = 1, rgba16f) uniform readonly image2D multiscatteringLUT;
layout (set = 2, binding = 2, rgba16f) uniform readonly image2D skyViewLUT;
layout (set = 2, binding = 3, rgba16f) uniform image3D AEPerspective;

/* One unit in global space should be 100 meters in camera coords */
const float cameraScale = 0.1;
struct ScatteringSample
{
    vec3 Mie;
    vec3 Ray;
};

/* ============================= PHASE FUNCTIONS ============================ */
float cornetteShanksMiePhaseFunction(float g, float cosTheta)
{
    float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + cosTheta * cosTheta) / pow(1.0 + g * g - 2.0 * g * -cosTheta, 1.5);
}

float rayleighPhase(float cosTheta)
{
    float factor = 3.0 / (16.0 * PI);
    return factor * (1.0 + cosTheta * cosTheta);
}
/* ========================================================================== */

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

ScatteringSample SampleMediumScattering(vec3 worldPosition)
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
    
    return ScatteringSample(mieScattering, rayleighScattering);
}
/* ========================================================================== */

vec3 getMultipleScattering(vec3 worldPosition, float viewZenithCosAngle)
{
    vec2 uv = clamp(vec2( 
        viewZenithCosAngle * 0.5 + 0.5,
        (length(worldPosition) - atmosphereParameters.bottom_radius) /
        (atmosphereParameters.top_radius - atmosphereParameters.bottom_radius)),
        0.0, 1.0);
    uv = vec2(fromUnitToSubUvs(uv.x, atmosphereParameters.MultiscatteringTexDimensions.x),
              fromUnitToSubUvs(uv.y, atmosphereParameters.MultiscatteringTexDimensions.y));
    ivec2 coords = ivec2(uv * atmosphereParameters.MultiscatteringTexDimensions);
    return imageLoad(multiscatteringLUT, coords).rgb;
}

struct RaymarchResult 
{
    vec3 Luminance;
    vec3 Transmittance;
};

RaymarchResult integrateScatteredLuminance(vec3 worldPosition, vec3 worldDirection, 
    vec3 sunDirection, int sampleCount, float maxDist)
{
    RaymarchResult result = RaymarchResult(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
    vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);

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
    integrationLength = min(integrationLength, maxDist);
    float cosTheta = dot(sunDirection, worldDirection);
    float miePhaseValue = cornetteShanksMiePhaseFunction(atmosphereParameters.mie_phase_function_g, -cosTheta);
    float rayleighPhaseValue = rayleighPhase(cosTheta);
    float oldRayShift = 0.0;
    float integrationStep = 0.0;

    vec3 accumTrans = vec3(1.0, 1.0, 1.0);
    vec3 accumLight = vec3(0.0, 0.0, 0.0);
    /* ============================= RAYMARCH ============================ */
    for(int i = 0; i < sampleCount; i++)
    {
        float newRayShift = integrationLength * (float(i) + 0.3) / sampleCount;
        integrationStep = newRayShift - oldRayShift;
        vec3 newPos = worldPosition + newRayShift * worldDirection;
        oldRayShift = newRayShift;

        ScatteringSample mediumScattering = SampleMediumScattering(newPos);
        vec3 mediumExtinction = SampleMediumExtinction(newPos);

        /* Raymarch shifts the angle to the sun a bit recalculate */
        vec3 upVector = normalize(newPos);
        vec2 transLUTParams = vec2( length(newPos),dot(sunDirection, upVector));

        /* uv coordinates later used to sample transmittance texture */
        vec2 transUV = TransmittanceLUTParamsToUv(transLUTParams, atmosphereBoundaries);
        /* because here transmittanceLUT is image and not a texture transfer
           from [0,1] -> [tex_width, tex_height] */
        ivec2 transImageCoords = ivec2(transUV * atmosphereParameters.TransmittanceTexDimensions);

        vec3 transmittanceToSun = vec3(imageLoad(transmittanceLUT, transImageCoords).rgb);
        vec3 phaseTimesScattering = mediumScattering.Mie * miePhaseValue + 
            mediumScattering.Ray * rayleighPhaseValue;

        float earthIntersectionDistance = raySphereIntersectNearest(
            newPos, sunDirection, planet0 + PLANET_RADIUS_OFFSET * upVector, atmosphereParameters.bottom_radius);
        float inEarthShadow = earthIntersectionDistance == -1.0 ? 1.0 : 0.0;

        vec3 multiscatteredLuminance = getMultipleScattering(newPos, dot(sunDirection, upVector)); 

        /* Light arriving from the sun to this point */
        vec3 sunLight = inEarthShadow * transmittanceToSun * phaseTimesScattering +
            multiscatteredLuminance * (mediumScattering.Ray + mediumScattering.Mie);

        /* TODO: This probably should be a texture lookup*/
        vec3 transIncreseOverInegrationStep = exp(-(mediumExtinction * integrationStep));
        vec3 sunLightInteg = (sunLight - sunLight * transIncreseOverInegrationStep) / mediumExtinction;
        accumLight += accumTrans * sunLightInteg;
        accumTrans *= transIncreseOverInegrationStep;
    }
    result.Luminance = accumLight;
    result.Transmittance = accumTrans;

    return result;
}

void main()
{

    vec3 camera = atmosphereParameters.camera_position;
    vec3 sun_direction = atmosphereParameters.sun_direction;

	mat4 invViewProjMat = inverse(commonParameters.lHviewProj);
    invViewProjMat = inverse(commonParameters.proj * commonParameters.view);
    vec2 pixPos = vec2(gl_GlobalInvocationID.xy + vec2(0.5, 0.5)) / atmosphereParameters.AEPerspectiveTexDimensions.xy;
    vec3 ClipSpace = vec3(pixPos*vec2(2.0, 2.0) - vec2(1.0, 1.0), 0.5);
    
    vec4 Hpos = invViewProjMat * vec4(ClipSpace, 1.0);
    vec3 worldDirection = normalize(Hpos.xyz / Hpos.w - camera);
    vec3 cameraPosition = camera  * cameraScale + vec3(0.0, 0.0, atmosphereParameters.bottom_radius);

    float Slice = ((float(gl_GlobalInvocationID.z) + 0.5) / atmosphereParameters.AEPerspectiveTexDimensions.z);
    Slice *= Slice;
    Slice *= atmosphereParameters.AEPerspectiveTexDimensions.z;

    /* TODO: Change slice size to be uniform variable */
    float tMax = Slice * 4.0;
    vec3 newWorldPos = cameraPosition + tMax * worldDirection;


    float viewHeight = length(newWorldPos);
    if (viewHeight <= (atmosphereParameters.bottom_radius + PLANET_RADIUS_OFFSET))
    {
        newWorldPos = normalize(newWorldPos) * (atmosphereParameters.bottom_radius + PLANET_RADIUS_OFFSET + 0.001);
        worldDirection = normalize(newWorldPos - cameraPosition);
        tMax = length(newWorldPos - cameraPosition);
    }

    viewHeight = length(cameraPosition);
    vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);
    if(viewHeight >= atmosphereParameters.top_radius)
    {
        vec3 prevWorldPos = cameraPosition;
        if(!moveToTopAtmosphere(cameraPosition, worldDirection, atmosphereBoundaries))
        {
            imageStore(AEPerspective, ivec3(gl_GlobalInvocationID.xyz), vec4( 0.0, 0.0, 0.0, 1.0));
            return;
        }
        float lengthToAtmosphere = length(prevWorldPos - cameraPosition);
        if(tMax < lengthToAtmosphere)
        {
            imageStore(AEPerspective, ivec3(gl_GlobalInvocationID.xyz), vec4( 0.0, 0.0, 0.0, 1.0));
            return;
        }
        tMax = max(0.0, tMax - lengthToAtmosphere);
    }
    int sampleCount = int(max(1.0, float(gl_GlobalInvocationID.z + 1.0) * 2.0));
    RaymarchResult res = integrateScatteredLuminance(cameraPosition, worldDirection, sun_direction,
        sampleCount, tMax);
    float averageTransmittance = (res.Transmittance.x + res.Transmittance.y + res.Transmittance.z) / 3.0;

    imageStore(AEPerspective, ivec3(gl_GlobalInvocationID.xyz), vec4(res.Luminance, averageTransmittance));
}