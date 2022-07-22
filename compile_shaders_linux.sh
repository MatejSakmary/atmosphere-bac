#!/bin/bash

mkdir -p shaders/build
mkdir -p shaders/build/noise

# =================================== Base triangle ===============================================================
glslc -fshader-stage=vert shaders/screen_triangle.glsl -I. -o shaders/build/screen_triangle.spv
# =================================== Terrain ===============================================================
glslc shaders/terrain.vert -I. -o shaders/build/terrain.vert.spv
glslc shaders/terrain.frag -I. -o shaders/build/terrain.frag.spv
# =================================== Sky LUTS ===============================================================
glslc -fshader-stage=comp shaders/transmittanceLUT.glsl     -I. -o shaders/build/transmittanceLUT.spv 
glslc -fshader-stage=comp shaders/multiscatteringLUT.glsl   -I. -o shaders/build/multiscatteringLUT.spv 
glslc -fshader-stage=comp shaders/skyviewLUT.glsl           -I. -o shaders/build/skyviewLUT.spv 
glslc -fshader-stage=comp shaders/aerialPerspectiveLUT.glsl -I. -o shaders/build/aerialPerspectiveLUT.spv 
# =================================== Noise ===============================================================
glslc -fshader-stage=comp shaders/noise/worley_noise_3D.glsl    -I. -o shaders/build/noise/worley_noise_3D.spv 
glslc -fshader-stage=comp shaders/noise/normalize_noise_3D.glsl -I. -o shaders/build/noise/normalize_noise_3D.spv 
# =================================== SkyDraw ===============================================================
glslc -fshader-stage=frag shaders/draw_far_sky.glsl        -I. -o shaders/build/draw_far_sky.spv 
glslc -fshader-stage=frag shaders/draw_clouds.glsl         -I. -o shaders/build/draw_clouds.spv 
glslc -fshader-stage=frag shaders/draw_AE_perspective.glsl -I. -o shaders/build/draw_AE_perspective.spv 
# =================================== Histogram ===============================================================
glslc -fshader-stage=comp shaders/histogram_generate.glsl   -I. -o shaders/build/histogram_generate.spv 
glslc -fshader-stage=comp shaders/histogram_sum.glsl        -I. -o shaders/build/histogram_sum.spv 
# =================================== Final Composition ===============================================================
glslc -fshader-stage=frag shaders/final_composition.glsl -I. -o shaders/build/final_composition.spv
echo "shaders compiled successfully"