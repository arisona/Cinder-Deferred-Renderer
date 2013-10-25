#pragma once
#include "cinder/CinderResources.h"

//images
#define NOISE_SAMPLER			CINDER_RESOURCE(../resources/textures/, random.png, 128, IMAGE)

//shaders
#define SHADER_SSUV_VERT		CINDER_RESOURCE(../resources/shaders/, ScreenSpaceUV.vert, 147, GLSL)

#define SHADER_SSAO_VERT		CINDER_RESOURCE(../resources/shaders/, SSAO_glsl.vert, 129, GLSL)
#define SHADER_SSAO_FRAG		CINDER_RESOURCE(../resources/shaders/, SSAO_glsl.frag, 130, GLSL)

#define SHADER_DEFER_VERT		CINDER_RESOURCE(../resources/shaders/, Deferred_glsl.vert, 131, GLSL)
#define SHADER_DEFER_FRAG		CINDER_RESOURCE(../resources/shaders/, Deferred_glsl.frag, 132, GLSL)

#define SHADER_BLENDER_VERT		CINDER_RESOURCE(../resources/shaders/, BasicBlender_glsl.vert, 133, GLSL)
#define SHADER_BLENDER_FRAG		CINDER_RESOURCE(../resources/shaders/, BasicBlender_glsl.frag, 134, GLSL)

#define SHADER_BLUR_H_VERT		CINDER_RESOURCE(../resources/shaders/, Blur_h_glsl.vert, 135, GLSL)
#define SHADER_BLUR_H_FRAG		CINDER_RESOURCE(../resources/shaders/, Blur_h_glsl.frag, 136, GLSL)

#define SHADER_BLUR_V_VERT		CINDER_RESOURCE(../resources/shaders/, Blur_v_glsl.vert, 137, GLSL)
#define SHADER_BLUR_V_FRAG		CINDER_RESOURCE(../resources/shaders/, Blur_v_glsl.frag, 138, GLSL)

#define SHADER_LIGHT_VERT		CINDER_RESOURCE(../resources/shaders/, Light_glsl.vert, 139, GLSL)
#define SHADER_LIGHT_FRAG		CINDER_RESOURCE(../resources/shaders/, Light_glsl.frag, 140, GLSL)

#define SHADER_ALPHA_RGB_VERT	CINDER_RESOURCE(../resources/shaders/, AlphaToRGB_glsl.vert, 141, GLSL)
#define SHADER_ALPHA_RGB_FRAG	CINDER_RESOURCE(../resources/shaders/, AlphaToRGB_glsl.frag, 142, GLSL)

#define SHADER_CUBESHADOW_VERT	CINDER_RESOURCE(../resources/shaders/, CubeShadowMap_glsl.vert, 143, GLSL)
#define SHADER_CUBESHADOW_FRAG	CINDER_RESOURCE(../resources/shaders/, CubeShadowMap_glsl.frag, 144, GLSL)

#define SHADER_FXAA_VERT		CINDER_RESOURCE(../resources/shaders/, FXAA_glsl.vert, 145, GLSL)
#define SHADER_FXAA_FRAG		CINDER_RESOURCE(../resources/shaders/, FXAA_glsl.frag, 146, GLSL)



//test Image
#define RES_EARTH_TEX                   CINDER_RESOURCE(../resources/, Earth_from_NASA.jpg, 148, IMAGE)


