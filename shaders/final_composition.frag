#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D texSampler; 
layout (std430, set = 1, binding = 0) buffer averageLuminance { float avgLum; };
/* layout (set = 2, binding = 0) */ #include "shaders/buffers/post_process_param_buff.glsl"

// Used to convert from linear RGB to XYZ space
const mat3 RGB_2_XYZ = (mat3(
    0.4124564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,
    0.1804375, 0.0721750, 0.9503041
));

// Used to convert from XYZ to linear RGB space
const mat3 XYZ_2_RGB = (mat3(
     3.2404542,-0.9692660, 0.0556434,
    -1.5371385, 1.8760108,-0.2040259,
    -0.4985314, 0.0415560, 1.0572252
));

// Converts a color from linear RGB to XYZ space
vec3 rgb_to_xyz(vec3 rgb) {
    return RGB_2_XYZ * rgb;
}

// Converts a color from XYZ to linear RGB space
vec3 xyz_to_rgb(vec3 xyz) {
    return XYZ_2_RGB * xyz;
}

// Converts a color from XYZ to xyY space (Y is luminosity)
vec3 xyz_to_xyY(vec3 xyz) {
    float Y = xyz.y;
    float x = xyz.x / (xyz.x + xyz.y + xyz.z);
    float y = xyz.y / (xyz.x + xyz.y + xyz.z);
    return vec3(x, y, Y);
}

// Converts a color from linear RGB to xyY space
vec3 rgb_to_xyY(vec3 rgb) {
    vec3 xyz = rgb_to_xyz(rgb);
    return xyz_to_xyY(xyz);
}

// Converts a color from xyY space to XYZ space
vec3 xyY_to_xyz(vec3 xyY) {
    float Y = xyY.z;
    float x = Y * xyY.x / xyY.y;
    float z = Y * (1.0 - xyY.x - xyY.y) / xyY.y;
    return vec3(x, Y, z);
}

// Converts a color from xyY space to linear RGB
vec3 xyY_to_rgb(vec3 xyY) {
    vec3 xyz = xyY_to_xyz(xyY);
    return xyz_to_rgb(xyz);
}

const float SRGB_ALPHA = 0.055;

// Converts a single linear channel to srgb
float linear_to_srgb(float channel) {
    if(channel <= 0.0031308)
        return 12.92 * channel;
    else
        return (1.0 + SRGB_ALPHA) * pow(channel, 1.0/2.4) - SRGB_ALPHA;
}

// Converts a linear rgb color to a srgb color (exact, not approximated)
vec3 rgb_to_srgb(vec3 rgb) {
    return vec3(
        linear_to_srgb(rgb.r),
        linear_to_srgb(rgb.g),
        linear_to_srgb(rgb.b)
    );
}

float Reinhard2(float x, float L_white) {
    return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

float Tonemap_ACES(float x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float Tonemap_Uchimura(float x, float P, float a, float m, float l, float c, float b) {
    // Uchimura 2017, "HDR theory and practice"
    // Math: https://www.desmos.com/calculator/gslcdxvipg
    // Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    float w0 = 1.0 - smoothstep(0.0, m, x);
    float w2 = step(m + l0, x);
    float w1 = 1.0 - w0 - w2;

    float T = m * pow(x / m, c) + b;
    float S = P - (P - S1) * exp(CP * (x - S0));
    float L = m + a * (x - m);

    return T * w0 + L * w1 + S * w2;
}

float Tonemap_Lottes(float x, float a, float d, float hdrMax, float midIn, float midOut) {

    // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
    // Can be precomputed
    const float b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const float c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, a) / (pow(x, a * d) * b + c);
}

void main()
{
    vec3 Col = texture(texSampler, inUV).rgb * 120000;
    vec3 xyY = rgb_to_xyY(Col);

    float lp = xyY.z / (9.6 * avgLum + 0.0001);
    if(postProcessParameters.tonemapCurve == 1)
    {
        xyY.z = Reinhard2(lp, postProcessParameters.whitepoint);
    } else if( postProcessParameters.tonemapCurve == 2) {
        xyY.z = Tonemap_ACES(lp);
    } else if( postProcessParameters.tonemapCurve == 3) {
        xyY.z = Tonemap_Uchimura(lp,
            postProcessParameters.maxDisplayBrigtness,
            postProcessParameters.contrast,
            postProcessParameters.linearSectionStart,
            postProcessParameters.linearSectionLength,
            postProcessParameters.black,
            postProcessParameters.pedestal);
    } else if( postProcessParameters.tonemapCurve == 4) {
        xyY.z = Tonemap_Lottes(lp,
        postProcessParameters.a,
        postProcessParameters.d,
        postProcessParameters.hdrMax,
        postProcessParameters.midIn,
        postProcessParameters.midOut);
    }

    Col = xyY_to_rgb(xyY);

    outColor = vec4(rgb_to_srgb(Col), 1.0);
    // vec3 white_point = vec3(1.08241, 0.96756, 0.95003) ;
    // float exposure = 10.0;
    //outColor = vec4(pow(vec3(1.0,1.0,1.0) - exp(-Col.rgb/white_point * exposure), vec3(1.0/ 2.2)), 1.0);
}