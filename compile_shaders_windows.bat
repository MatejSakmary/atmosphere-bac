@echo off
glslc shaders/screen_triangle.vert -o shaders/screen_triangle.vert.spv
glslc shaders/compute_transmittance_LUT.comp -o shaders/compute_transmittance_LUT.comp.spv
glslc shaders/compute_multiple_scattering_LUT.comp -o shaders/compute_multiple_scattering_LUT.comp.spv 
glslc shaders/compute_sky_view_LUT.comp -o shaders/compute_sky_view_LUT.comp.spv 
glslc shaders/compute_frag_test.frag -o shaders/compute_frag_test.frag.spv
glslc shaders/terrain.vert -o shaders/terrain.vert.spv
glslc shaders/terrain.frag -o shaders/terrain.frag.spv
glslc shaders/final_composition.frag -o shaders/final_composition.frag.spv

if not exist shaders/build (
    mkdir .\shaders\build
    echo made shader build dir
) 
if not exist shaders/build/MaSa_shaders (
    mkdir .\shaders\build\MaSa_shaders
    echo made MaSa_shaders build dir
)
if not exist shaders/build/MaSa_shaders/WorleyNoise3D (
    mkdir .\shaders\build\MaSa_shaders\WorleyNoise3D
    echo made WorleyNoise3D build dir
)

:: =================================== Sky LUTS ===============================================================
glslc -fshader-stage=comp shaders/MaSa_shaders/transmittanceLUT.glsl -I. -o shaders/build/MaSa_shaders/transmittanceLUT.spv 
glslc -fshader-stage=comp shaders/MaSa_shaders/multiscatteringLUT.glsl -I. -o shaders/build/MaSa_shaders/multiscatteringLUT.spv 
glslc -fshader-stage=comp shaders/MaSa_shaders/skyviewLUT.glsl -I. -o shaders/build/MaSa_shaders/skyviewLUT.spv 
glslc -fshader-stage=comp shaders/MaSa_shaders/aerialPerspectiveLUT.glsl -I. -o shaders/build/MaSa_shaders/aerialPerspectiveLUT.spv 
:: =================================== Histogram ===============================================================
glslc -fshader-stage=comp shaders/MaSa_shaders/histogram_generate.glsl -I. -o shaders/build/MaSa_shaders/histogram_generate.spv 
glslc -fshader-stage=comp shaders/MaSa_shaders/histogram_sum.glsl -I. -o shaders/build/MaSa_shaders/histogram_sum.spv 
:: =================================== Noise ===============================================================
glslc -fshader-stage=comp shaders/MaSa_shaders/WorleyNoise3D/worley_noise_3D.glsl -I. -o shaders/build/MaSa_shaders/WorleyNoise3D/worley_noise_3D.spv 
glslc -fshader-stage=comp shaders/MaSa_shaders/WorleyNoise3D/normalize_noise_3D.glsl -I. -o shaders/build/MaSa_shaders/WorleyNoise3D/normalize_noise_3D.spv 
:: =================================== SkyDraw ===============================================================
glslc -fshader-stage=frag shaders/MaSa_shaders/draw_far_sky.glsl -I. -o shaders/build/MaSa_shaders/draw_far_sky.spv 
glslc -fshader-stage=frag shaders/MaSa_shaders/draw_clouds.glsl -I. -o shaders/build/MaSa_shaders/draw_clouds.spv 
glslc -fshader-stage=frag shaders/MaSa_shaders/draw_AE_perspective.glsl -I. -o shaders/build/MaSa_shaders/draw_AE_perspective.spv 
echo compiling done
