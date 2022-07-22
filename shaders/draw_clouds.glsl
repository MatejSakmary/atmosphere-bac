#version 450

#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec2 inUV;

#include "shaders/common_func.glsl"

/* layout (set = 0, binding = 0) */ #include "shaders/buffers/common_param_buff.glsl"
/* layout (set = 1, binding = 0) */ #include "shaders/buffers/atmosphere_param_buff.glsl"
/* layout (set = 2, binding = 0  */ #include "shaders/buffers/clouds_param_buffer.glsl"
layout (input_attachment_index = 0, set = 3, binding = 0) uniform subpassInput depthInput; 
layout (set = 4, binding = 0) uniform sampler3D worleyNoiseSampler;
layout (set = 4, binding = 1) uniform sampler3D worleyNoiseDetailSampler;
layout (set = 5, binding = 0, rgba16f) uniform readonly image2D transmittanceLUT;

/* One unit in global space should be 100 meters in camera coords */
const float cameraScale = 0.1;

// Henyey-Greenstein
float hg(float a, float g) {
    float g2 = g*g;
    return (1-g2) / (4*3.1415*pow(1+g2-2*g*(a), 1.5));
}

float phase(float a) {
    vec4 phaseParams = cloudsParameters.phaseParams;
    float blend = phaseParams.w;
    float hgBlend = hg(a,phaseParams.x) * (1-blend) + hg(a,-phaseParams.y) * blend;
    return phaseParams.z + hgBlend;
}

/* ========================= OPEN SPACE ================================== */
vec3 miePhaseFunctionDHG(float mu, vec3 g1, vec3 g2, vec3 alpha) {
  // Double Henyey-Greenstein phase function
  vec3 g1Sq = g1 * g1;
  vec3 g2Sq = g2 * g2;
  const float exponent = 3.0 / 2.0;

  vec3 denom1 = vec3(1.0 + g1Sq - 2.0 * g1 * mu);
  vec3 denom2 = vec3(1.0 + g2Sq - 2.0 * g2 * mu);
  vec3 d1Powered = vec3(pow(denom1.r, exponent), pow(denom1.g, exponent), pow(denom1.b, exponent));
  vec3 d2Powered = vec3(pow(denom2.r, exponent), pow(denom2.g, exponent), pow(denom2.b, exponent));

  vec3 f1 = ((vec3(1.0) - g1Sq) / d1Powered);
  vec3 f2 = ((vec3(1.0) - g2Sq) / d2Powered);
  return (alpha * f1 + (vec3(1.0) - alpha) * f2) * 0.25 / PI;
}


// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
    // Adapted from: http://jcgt.org/published/0007/03/04/
    vec3 t0 = /*-rayOrigin * invRaydir - invRaydir * boundsMin;*/(boundsMin - rayOrigin) * invRaydir;
    vec3 t1 = /*-rayOrigin * invRaydir - invRaydir * boundsMax;*/(boundsMax - rayOrigin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));
    
    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection
    
    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection
    
    // CASE 3: ray misses box (dstA > dstB)
    
    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

float remap(float v, float minOld, float maxOld, float minNew, float maxNew) {
    return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

float saturate(float x) { return clamp(x, 0.0, 1.0); }

float sampleDensity(vec3 samplePos, float distFactor)
{
    const float baseScale = 1.0/1000.0;
    vec3 uvw = samplePos * baseScale * cloudsParameters.cloudsScale * vec3(1.0, 1.0, 1.0);

    float heightGradient = 1.0;
    float cloudLayerThickness = cloudsParameters.maxBounds - cloudsParameters.minBounds;
    float heightAboveGround = length((samplePos * cameraScale) + 
        vec3(0.0, 0.0, atmosphereParameters.bottom_radius)) - atmosphereParameters.bottom_radius;
    float percentInCloudLayer = max((heightAboveGround - cloudsParameters.minBounds), 0.0) / cloudLayerThickness; 

    if(cloudsParameters.debug == 1)
    {
        // if(percentInCloudLayer < 0.2) { heightGradient = mix(0.0, 1.0, percentInCloudLayer * 5.0); } 
        // else if (percentInCloudLayer > 0.8) { heightGradient = 1.0 - smoothstep(0.7, 1.0, percentInCloudLayer); }
        const float a = cloudsParameters.minBounds;
        const float h = cloudsParameters.maxBounds;
        heightGradient = (heightAboveGround - a) * (heightAboveGround - h - a) * (-4/(h * h));
    }

    vec4 shapeNoise = texture(worleyNoiseSampler, uvw);
    vec4 normalizedShapeWeights = normalize(cloudsParameters.shapeNoiseWeights); 
    float shapeFBM = dot(shapeNoise, normalizedShapeWeights);// * heightGradient;
    float baseShapeDensity = shapeFBM - min(cloudsParameters.densityOffset + distFactor, 1.0);
    if(baseShapeDensity > 0.0)
    {
        vec3 detailSamplePos = uvw * cloudsParameters.detailScale;
        vec4 detailNoise = texture(worleyNoiseDetailSampler, detailSamplePos);
        vec4 normalizedDetailWeights = normalize(cloudsParameters.detailNoiseWeights);
        float detailFBM = dot(detailNoise, normalizedDetailWeights);

        float oneMinusShape = 1.0 - baseShapeDensity;
        float detailErodeWeight = 0.0;
        detailErodeWeight = oneMinusShape * oneMinusShape * oneMinusShape;

        float cloudDensity = baseShapeDensity - (1.0 - detailFBM) * detailErodeWeight * cloudsParameters.detailNoiseMultiplier;
        return cloudDensity * cloudsParameters.densityMultiplier * 4.9 * heightGradient;
    }
    return 0.0;
}

vec2 getRayCloudLayerInfo(float cloudAltMin, float cloudAltMax, vec3 position, vec3 rayDirection)
{
    /* Get position offset by the radius of the planet */
    vec3 planetPosition = position + vec3(0.0, 0.0, atmosphereParameters.bottom_radius);
    vec2 cloudLayerBoundaries = vec2(cloudAltMin, cloudAltMax) + vec2(atmosphereParameters.bottom_radius);

    /* vec4(distToCloudLayer0, distThroughCloudLayer0, distToCloudLayer1, distThroughCloudLayer1) */
    vec2 result = vec2(-1.0, -1.0);
    vec3 planet0 = vec3(0.0, 0.0, 0.0);
    float bottomCID = raySphereIntersectNearest(
        planetPosition, rayDirection, planet0, cloudLayerBoundaries.x);
    float topCID = raySphereIntersectNearest(
        planetPosition, rayDirection, planet0, cloudLayerBoundaries.y);
    float groundCID = raySphereIntersectNearest(
        planetPosition, rayDirection, planet0, atmosphereParameters.bottom_radius);

    /* Above clouds */
    if(length(planetPosition) >= cloudLayerBoundaries.y)
    {
        if(topCID == -1.0) {return result;}
        if(bottomCID == -1.0)
        {
            vec3 newPosition = planetPosition + (topCID.x + 0.001) * rayDirection;
            float topCID2 = raySphereIntersectNearest(newPosition, rayDirection, planet0, cloudLayerBoundaries.y);
            if(topCID2 == -1.0) {return result;}
            result.x = topCID;
            result.y = topCID2 - topCID;
            return result;
        }else{
            result.x = topCID;
            result.y = bottomCID - topCID;
            return result;
        }
    }
    /* In clouds */
    else if ( length(planetPosition) < cloudLayerBoundaries.y && 
              length(planetPosition) > cloudLayerBoundaries.x )
    {
        result.x = 0.0;
        if (topCID < 0.0)         { result.y = bottomCID; }
        else if (bottomCID < 0.0) { result.y = topCID; } 
        else                      { result.y = min(topCID, bottomCID);}
        return result;
    }
    /* Under clouds */
    else if (length(planetPosition) <= cloudLayerBoundaries.x)
    {
        if(groundCID == -1.0)
        {
           result.x = bottomCID;
           result.y = topCID - bottomCID;
        } 
        return result;
    }
}

float getCloudTransAlongRay(vec3 startPos)
{
    vec3 dirToLight = atmosphereParameters.sun_direction;
    // float integrationLength = rayBoxDst(cloudsParameters.minBounds, cloudsParameters.maxBounds,
    //     startPos, 1/dirToLight).y;
    float integrationLength = getRayCloudLayerInfo(cloudsParameters.minBounds,
        cloudsParameters.maxBounds, startPos * cameraScale, dirToLight).y * 1/cameraScale;
    
    integrationLength = min(integrationLength, 20.0);

    float oldRayShift = 0.0;
    float totalDensity = 0.0;
    for(int i = 0; i < cloudsParameters.sampleCountToSun; i++)
    {
        float step_0 = float(i) / cloudsParameters.sampleCountToSun;
        float step_1 = float(i + 1) / cloudsParameters.sampleCountToSun;

        step_0 *= step_0;
        step_1 *= step_1;

        step_0 = step_0 * integrationLength;
        step_1 = step_1 > 1.0 ? integrationLength : step_1 * integrationLength;

        float newRayShift = step_0 + (step_1 - step_0) * 0.3;
        float integrationStep = step_1 - step_0;
        vec3 newPos = startPos + newRayShift * dirToLight;

        // float newRayShift = integrationLength * (float(i) + 0.3) / cloudsParameters.sampleCount;
        // float integrationStep = newRayShift - oldRayShift;
        // vec3 newPos = startPos + newRayShift * dirToLight;
        oldRayShift = newRayShift;
        totalDensity += max(0.0, sampleDensity(newPos, 0.0) * integrationStep);
    }

    float transmittance = exp(-totalDensity * cloudsParameters.lightAbsTowardsSun);
    return cloudsParameters.darknessThreshold + transmittance * (1.0 - cloudsParameters.darknessThreshold);
}

void main()
{
    vec4 L = vec4(1.0, 0.0, 0.0, 1.0);
    /* Camera position in world space */
    vec3 cameraPosition = atmosphereParameters.camera_position;

    /* sample depth from depth map used in rendering the terrain  */
    float depth = subpassLoad(depthInput).r;
    gl_FragDepth = depth;
    /* Vulkans clip space range [-1, -1] - (1, 1) -> z is depth [0, 1] */
    vec3 clipSpace = vec3(inUV * vec2(2.0) - vec2(1.0), depth);
    /* inverse view projection matrix which gets us from clip space to world space */
    mat4 invViewProjMat = inverse(commonParameters.proj * commonParameters.view);
    vec4 hPos = invViewProjMat * vec4(clipSpace, 1.0);
    /* Get unit lenght ray from camera origin to processed fragment in world space */
    vec3 cameraRayWorld = normalize(hPos.xyz/hPos.w - cameraPosition);


    /* Get depth in world space */
    float realDepth = length(hPos.xyz/hPos.w - cameraPosition);

    float sunRayCosAngle = dot(cameraRayWorld, atmosphereParameters.sun_direction);
    float phaseValue = phase(sunRayCosAngle);

    vec2 rayToCloudLayerInfo = getRayCloudLayerInfo(cloudsParameters.minBounds,
        cloudsParameters.maxBounds, cameraPosition * cameraScale, cameraRayWorld);
    float distanceToCloudBB = rayToCloudLayerInfo.x * 1.0/cameraScale;
    float distanceInsideCloudBB = rayToCloudLayerInfo.y * 1.0/cameraScale;

    /* Make sure to only integrate the visible part of clouds -> if there is f.e.
       a hill obstructing part of (hidden inside) the cloud BB
       raymarch only towards the start of that hill */
    float integrationLength = min(realDepth - distanceToCloudBB, distanceInsideCloudBB);

    if(integrationLength <= 0 )
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    /* TODO: This probably should be blueNoise */
    float offset = fract(sin(dot(inUV, vec2(12.9898, 78.233))) * 43758.5453) * integrationLength/70.0;
    // float offset = 0.0;
    /* Offset to start raymarch at the start of the cloud layer */
    vec3 startPosition = cameraPosition + (distanceToCloudBB + offset) * cameraRayWorld;
    integrationLength -= length(offset * cameraRayWorld);

    vec3 depthBufferPosition = vec3(0.0, 0.0, 0.0);
    
    float oldRayShift = 0.0;
    float transmittance = 1.0;
    float powderTrans = 1.0;
    vec3 lightEnergy = vec3(0.0);
    float accumLinearDepth = 0.0;
    float accumTransmittanceSum = 0.0;
    for(int i = 0; i < cloudsParameters.sampleCount; i++)
    {
        if(transmittance < 0.01)
        {
            break;
        }
        #define USE_LINEAR_SAMPLING 1
        #if USE_LINEAR_SAMPLING
        float newRayShift = integrationLength * (float(i) + 0.3) / cloudsParameters.sampleCount;
        float integrationStep = newRayShift - oldRayShift;
        vec3 newPos = startPosition + newRayShift * cameraRayWorld;
        oldRayShift = newRayShift;
        #else
        float step_0 = float(i) / cloudsParameters.sampleCount;
        float step_1 = float(i + 1) / cloudsParameters.sampleCount;

        step_0 *= step_0;
        step_1 *= step_1;

        step_0 = step_0 * integrationLength;
        step_1 = step_1 > 1.0 ? integrationLength : step_1 * integrationLength;

        float newRayShift = step_0 + (step_1 - step_0) * 0.3;
        float integrationStep = step_1 - step_0;
        vec3 newPos = startPosition + newRayShift * cameraRayWorld;
        #endif

        float density = sampleDensity(newPos, 0.0);

        if(density > 0)
        {
            if(transmittance < 1.0) {
                depthBufferPosition = newPos; 
                vec4 projPos = commonParameters.proj * commonParameters.view * vec4(depthBufferPosition, 1.0);
                float linearDepthSample = projPos.z / projPos.w;
                accumLinearDepth += transmittance * linearDepthSample;
                accumTransmittanceSum += transmittance;
            }
            float transmittanceToSun = getCloudTransAlongRay(newPos);

            float transIncreseOverInegrationStep = exp(-density * integrationStep * cloudsParameters.lightAbsThroughCloud);
            float powderTransmittanceIncOverIntStep= exp(-density * integrationStep * cloudsParameters.lightAbsThroughCloud * 2.0);
            float sunLight = transmittanceToSun * phaseValue * density;
            float sunLightInt = (sunLight - sunLight * transIncreseOverInegrationStep * powderTransmittanceIncOverIntStep) / density;

            vec3 realWorldPos = newPos * cameraScale + vec3(0.0, 0.0, atmosphereParameters.bottom_radius);
            float height = length(realWorldPos);
            vec3 upVector = realWorldPos/height;
            float viewZenithCosAngle = dot(atmosphereParameters.sun_direction, upVector);
            vec2 transLUTParams = vec2( height, viewZenithCosAngle);
            vec2 atmosphereBoundaries = vec2(atmosphereParameters.bottom_radius, atmosphereParameters.top_radius);
            vec2 transUV = TransmittanceLUTParamsToUv(transLUTParams, atmosphereBoundaries); 
            ivec2 transImageCoords = ivec2(transUV * atmosphereParameters.TransmittanceTexDimensions);
            vec3 transmittanceToSunAtmo = vec3(imageLoad(transmittanceLUT, transImageCoords).rgb);

            lightEnergy += sunLightInt * transmittance * transmittanceToSunAtmo;
            
            transmittance *= transIncreseOverInegrationStep * powderTransmittanceIncOverIntStep;
        }
    }

    if(depthBufferPosition == vec3(0.0, 0.0, 0.0))
    {
        gl_FragDepth = depth;
    } else 
    {
        float cloudDepth = accumLinearDepth/accumTransmittanceSum;
        gl_FragDepth = mix(cloudDepth, depth, pow(transmittance, 2));
    }

    vec3 cloudCol = lightEnergy * vec3(1.0, 1.0, 1.0) * 0.05;
    float transmittanceDistIncr = clamp((distanceToCloudBB - 400.0) / 2000.0, 0.0, 1.0);
    transmittance = clamp(transmittance + transmittanceDistIncr, 0.0, 1.0);
    cloudCol *= float(1.0 - transmittanceDistIncr); //

    // transmittance = min(transmittance + min(max(distanceToCloudBB - 100.0, 0.0) / 200.0, 1.0),1.0);
    // cloudCol *= (1.0 - min(max(distanceToCloudBB - 100.0, 0.0) / 200.0, 1.0),1.0);

    outColor = vec4(cloudCol, transmittance);
}
