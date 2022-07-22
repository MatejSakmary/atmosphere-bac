layout(set = 1, binding = 0) uniform AtmosphereParametersBuffer
{
    vec3 solar_irradiance;
    float sun_angular_radius;

    vec3 absorption_extinction;

    vec3 rayleigh_scattering; 
    float mie_phase_function_g;

    vec3 mie_scattering; 
    float bottom_radius;

    vec3 mie_extinction; 
    float top_radius;

    vec3 mie_absorption;
    vec3 ground_albedo;

    vec4 rayleigh_density[3];
    vec4 mie_density[3];
    vec4 absorption_density[3];

    vec2 TransmittanceTexDimensions;
    vec2 MultiscatteringTexDimensions;
    vec2 SkyViewTexDimensions;
    vec3 AEPerspectiveTexDimensions;

    vec3 sun_direction;
    vec3 camera_position;
} atmosphereParameters;