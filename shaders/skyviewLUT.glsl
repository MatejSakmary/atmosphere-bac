#version 450
layout (local_size_x = 16, local_size_y = 16) in;

#extension GL_GOOGLE_include_directive : require
#include "shaders/common_func.glsl"

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
layout (set = 2, binding = 0, rgba16f) uniform readonly image2D transmittanceLUT;
layout (set = 2, binding = 1, rgba16f) uniform readonly image2D multiscatteringLUT;
layout (set = 2, binding = 2, rgba16f) uniform image2D skyViewLUT;
/* ================================== NOT USED ==================================== */
layout (set = 2, binding = 3, rgba16f) uniform readonly image3D AEPerspective;
/* ================================================================================ */

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

vec3 integrateScatteredLuminance(vec3 worldPosition, vec3 worldDirection, 
    vec3 sunDirection, int sampleCount)
{
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
        return vec3(0.0, 0.0, 0.0);
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

    float cosTheta = dot(sunDirection, worldDirection);
    float miePhaseValue = cornetteShanksMiePhaseFunction(atmosphereParameters.mie_phase_function_g, -cosTheta);
    float rayleighPhaseValue = rayleighPhase(cosTheta);

    vec3 accumTrans = vec3(1.0, 1.0, 1.0);
    vec3 accumLight = vec3(0.0, 0.0, 0.0);
    /* ============================= RAYMARCH ============================ */
    for(int i = 0; i < sampleCount; i++)
    {
        /* Step size computation */
        float step_0 = float(i) / sampleCount;
        float step_1 = float(i + 1) / sampleCount;

        /* Nonuniform step size*/
        step_0 *= step_0;
        step_1 *= step_1;

        step_0 = step_0 * integrationLength;
        step_1 = step_1 > 1.0 ? integrationLength : step_1 * integrationLength;
        /* Sample at one third of the integrated interval -> better results for
           exponential functions */
        float integrationStep = step_0 + (step_1 - step_0) * 0.3;
        float dIntStep = step_1 - step_0;

        /* Position shift */
        vec3 newPos = worldPosition + integrationStep * worldDirection;
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
        vec3 transIncreseOverInegrationStep = exp(-(mediumExtinction * dIntStep));
        vec3 sunLightInteg = (sunLight - sunLight * transIncreseOverInegrationStep) / mediumExtinction;
        accumLight += accumTrans * sunLightInteg;
        accumTrans *= transIncreseOverInegrationStep;
    }
    return accumLight;
}

void main()
{
    /* TODO: probably should be a vec2 in the buffer in the first place */
    vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);
    /* TODO: change to represent real camera position */
    const float cameraHeight = atmosphereParameters.camera_position.z * cameraScale;
    /* TODO: make sun direction depend on parameters in atmosphereParameters buffer */
    vec3 sunDirection = atmosphereParameters.sun_direction;
    vec3 worldPosition = vec3(0.0, 0.0, cameraHeight + atmosphereParameters.bottom_radius);

    vec2 uv = vec2(gl_GlobalInvocationID.xy) / atmosphereParameters.SkyViewTexDimensions;
    vec2 LUTParams = UvToSkyViewLUTParams(uv, atmosphereBoundaries,
        atmosphereParameters.SkyViewTexDimensions, length(worldPosition));

    float sunZenithCosAngle = dot(normalize(worldPosition), sunDirection);
    vec3 localSunDirection = normalize(vec3(
        safeSqrt(1.0 - sunZenithCosAngle * sunZenithCosAngle),
        0.0,
        sunZenithCosAngle));
    
    float viewZenithSinAngle = safeSqrt(1.0 - LUTParams.x * LUTParams.x);
    vec3 worldDirection = vec3(
        cos(LUTParams.y) * sin(LUTParams.x),
        sin(LUTParams.y) * sin(LUTParams.x),
        cos(LUTParams.x));
    
    if (!moveToTopAtmosphere(worldPosition, worldDirection, atmosphereBoundaries))
    {
        /* No intersection with the atmosphere */
        imageStore(skyViewLUT, ivec2(gl_GlobalInvocationID.xy), vec4(0.0, 0.0, 0.0, 1.0));
        return;
    }
    vec3 Luminance = integrateScatteredLuminance(worldPosition, worldDirection, localSunDirection, 30);
    imageStore(skyViewLUT, ivec2(gl_GlobalInvocationID.xy), vec4(Luminance, 1.0));
}