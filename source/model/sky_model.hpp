#pragma once
#define GLM_FORCE_RADIANS
/* GLM uses OpenGL default of -1,1 for the perspective projection so
   I need to force it to use Vulkan default which is 0,1 */
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

struct AtmosphereParametersBuffer
{
    alignas(16) glm::vec3 solar_irradiance;
    alignas(4) float sun_angular_radius;

    alignas(16) glm::vec3 absorption_extinction;

    alignas(16) glm::vec3 rayleigh_scattering; 
    alignas(4) float mie_phase_function_g;

    alignas(16) glm::vec3 mie_scattering; 
    alignas(4) float bottom_radius;

    alignas(16) glm::vec3 mie_extinction; 
    alignas(4) float top_radius;

    alignas(16) glm::vec3 mie_absorption;
    alignas(16) glm::vec3 ground_albedo;

    alignas(16) float rayleigh_density[12];
    alignas(16) float mie_density[12];
    alignas(16) float absorption_density[12];

    alignas(8) glm::vec2 TransmittanceTexDimensions;
    alignas(8) glm::vec2 MultiscatteringTexDimensions;
    alignas(8) glm::vec2 SkyViewTexDimensions;
    alignas(16) glm::vec3 AEPerspectiveTexDimensions;

    alignas(16) glm::vec3 sunDirection;
    alignas(16) glm::vec3 cameraPosition;
    alignas(4) float sunPhiAngle;
    alignas(4) float sunThetaAngle;
};

void SetupAtmosphereParametersBuffer(AtmosphereParametersBuffer& buffer);