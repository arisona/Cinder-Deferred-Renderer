//
//  DeferredRenderer.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

#pragma once

#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"

#include "PointLight.h"
#include "CubeShadowMap.h"

using namespace ci;
using namespace ci::app;

class DeferredRenderer {
private:
	std::function<void(gl::GlslProg*)> mRenderShadowCastersFunc;
	std::function<void(gl::GlslProg*)> mRenderNonShadowCastersFunc;

	MayaCamUI* mCamera;
	Vec2i mFBOResolution;
	int mShadowMapResolution;

	gl::Texture mRandomNoise;

	gl::GlslProg mDeferredShader;
	gl::GlslProg mCubeShadowShader;
	gl::GlslProg mLightShader;
	gl::GlslProg mSSAOShader;
	gl::GlslProg mHBlurShader;
	gl::GlslProg mVBlurShader;
	gl::GlslProg mBlenderShader;
	gl::GlslProg mFXAAShader;
	gl::GlslProg mAlphaToRBGShader;

	gl::Fbo mDeferredFBO;
	gl::Fbo mShadowFBO;
	gl::Fbo mLightFBO;
	gl::Fbo mSSAOFBO;
	gl::Fbo	mHBlurFBO;
	gl::Fbo mVBlurFBO;
	gl::Fbo mFinalFBO;
	
	// for shadowing
	Matrix44f mLightFaceViewMatrices[6];
    CubeShadowMap mCubeShadowMap;
    gl::Fbo mCubeDepthFBO;

	std::vector<PointLight*> mLights;

public:
	enum RenderMode {
		SHOW_FINAL_VIEW,
		SHOW_DIFFUSE_VIEW,
		SHOW_NORMALMAP_VIEW,
		SHOW_DEPTH_VIEW,
		SHOW_POSITION_VIEW,
		SHOW_ATTRIBUTE_VIEW,
		SHOW_SSAO_VIEW,
		SHOW_SSAO_BLURRED_VIEW,
		SHOW_LIGHT_VIEW,
		SHOW_SHADOWS_VIEW
	};
	
public:
	DeferredRenderer() {};
	~DeferredRenderer() {};
	
	void setup(const std::function<void(gl::GlslProg*)> renderShadowCastersFunc,
			   const std::function<void(gl::GlslProg*)> renderNonShadowCastersFunc,
			   MayaCamUI* camera,
			   Vec2i FBOResolution = Vec2i(512, 512),
			   int shadowMapResolution = 512) {
		mRenderShadowCastersFunc = renderShadowCastersFunc;
		mRenderNonShadowCastersFunc = renderNonShadowCastersFunc;
		mCamera = camera;
		mFBOResolution = FBOResolution;
		mShadowMapResolution = shadowMapResolution;
		
		glClearDepth(1.0f);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glShadeModel(GL_SMOOTH);
		glDisable(GL_LIGHTING);
		glClearColor(0, 0, 0, 0);
		glColor4d(1, 1, 1, 1);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		
		//---- init textures
		mRandomNoise = gl::Texture(loadImage(loadResource(NOISE_SAMPLER))); //noise texture required for SSAO calculations
		
		//---- init shaders
		DataSourceRef ssuvVert = loadResource(SHADER_SSUV_VERT);
		
		mDeferredShader		= gl::GlslProg(loadResource(SHADER_DEFER_VERT), loadResource(SHADER_DEFER_FRAG));
		mCubeShadowShader	= gl::GlslProg(loadResource(SHADER_CUBESHADOW_VERT), loadResource(SHADER_CUBESHADOW_FRAG));
		mLightShader		= gl::GlslProg(loadResource(SHADER_LIGHT_VERT), loadResource(SHADER_LIGHT_FRAG));
		mSSAOShader			= gl::GlslProg(ssuvVert, loadResource(SHADER_SSAO_FRAG));
		mHBlurShader		= gl::GlslProg(ssuvVert, loadResource(SHADER_BLUR_H_FRAG));
		mVBlurShader		= gl::GlslProg(ssuvVert, loadResource(SHADER_BLUR_V_FRAG));
		mBlenderShader		= gl::GlslProg(ssuvVert, loadResource(SHADER_BLENDER_FRAG));
		mFXAAShader			= gl::GlslProg(ssuvVert, loadResource(SHADER_FXAA_FRAG));

		mAlphaToRBGShader	= gl::GlslProg(ssuvVert, loadResource(SHADER_ALPHA_RGB_FRAG));
		
		//---- init FBOs
		
		// special fbo format for deferred buffer
		// 4 color attachments:
		// - diffuse color (rgba)
		// - normal + depth (rgb + a)
		// - position (rgb + a ignored)
		// - diff_coeff + phong_coeff + two_sides (rgb + a ignored)
		// XXX: currently, the depth buffer isn't used (depth stored in color attachment)
		gl::Fbo::Format mtRFBO;
		//mtRFBO.enableDepthBuffer();
		//mtRFBO.setDepthInternalFormat(GL_DEPTH_COMPONENT32);
		mtRFBO.setColorInternalFormat(GL_RGBA16F_ARB);
		mtRFBO.enableColorBuffer(true, 4);
		//mtRFBO.setSamples(4); // enable 4x antialiasing
		
		// standard fbo format for all others
		gl::Fbo::Format format;
		//format.setDepthInternalFormat(GL_DEPTH_COMPONENT32);
		//format.setColorInternalFormat(GL_RGBA16F_ARB);
		//format.setSamples(4); // enable 4x antialiasing
		
		// init screen space render
		mDeferredFBO = gl::Fbo(mFBOResolution.x, mFBOResolution.y, mtRFBO);
		mShadowFBO = gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		mLightFBO = gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		mSSAOFBO = gl::Fbo(mFBOResolution.x / 2, mFBOResolution.y / 2, format);
		mHBlurFBO = gl::Fbo(mFBOResolution.x / 2, mFBOResolution.y / 2, format);
		mVBlurFBO = gl::Fbo(mFBOResolution.x / 2, mFBOResolution.y / 2, format);
		mFinalFBO = gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		
		
		//---- init cube shadow fbos
		mCubeShadowMap.setup(mShadowMapResolution);
		
		// create FBO to hold depth values from cube map
		gl::Fbo::Format formatCube;
		formatCube.enableColorBuffer(true, 1);
		formatCube.enableDepthBuffer(true, true);
		formatCube.setMinFilter(GL_LINEAR);
		formatCube.setMagFilter(GL_LINEAR);
		formatCube.setWrap(GL_CLAMP, GL_CLAMP);
		mCubeDepthFBO = gl::Fbo(mShadowMapResolution, mShadowMapResolution, formatCube);
		
		//---- init shadow cube: axial matrices required for six-sides of calculations for cube shadows
		CameraPersp cubeCam;
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(1, 0, 0), Vec3f(0, -1, 0));
		mLightFaceViewMatrices[CubeShadowMap::X_FACE_POS] = cubeCam.getModelViewMatrix();
		
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(-1, 0, 0), Vec3f(0, -1, 0));
		mLightFaceViewMatrices[CubeShadowMap::X_FACE_NEG] = cubeCam.getModelViewMatrix();
		
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(0, 1, 0), Vec3f(0, 0, 1));
		mLightFaceViewMatrices[CubeShadowMap::Y_FACE_POS] = cubeCam.getModelViewMatrix();
		
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(0.0,-1.0, 0.0), Vec3f(0.0, 0.0,-1.0));
		mLightFaceViewMatrices[CubeShadowMap::Y_FACE_NEG] = cubeCam.getModelViewMatrix();
		
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(0, 0, 1), Vec3f(0, -1, 0));
		mLightFaceViewMatrices[CubeShadowMap::Z_FACE_POS] = cubeCam.getModelViewMatrix();
		
		cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(0, 0, -1), Vec3f(0, -1, 0));
		mLightFaceViewMatrices[CubeShadowMap::Z_FACE_NEG] = cubeCam.getModelViewMatrix();
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	void render(RenderMode renderMode, bool enableShadows, bool enableSSAO) {
		// 1. render main scene to FBO
		
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		gl::setMatrices(mCamera->getCamera());
		renderSceneToDeferredFBO();
		
		// 2. create shadow maps and render shadows to FBOs
		
		if (enableShadows) renderShadowsToFBOs();
		
		// 3. render lights & ssao to FBO
		
		gl::setMatrices(mCamera->getCamera());
		renderLightsToFBO();
		renderSSAOToFBO();
		
		// 4. final deferred rendering, depending on current mode
		
		switch (renderMode) {
			case SHOW_FINAL_VIEW: // key 0
				renderPingPongBlurToFBO();
				
				mFinalFBO.bindFramebuffer();
				glClearColor(0.5f, 0.5f, 0.5f, 1);
				glClearDepth(1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				gl::setMatricesWindow((float)mFinalFBO.getWidth(), (float)mFinalFBO.getHeight());
				gl::setViewport(mFinalFBO.getBounds());
				
				if (enableSSAO) mVBlurFBO.getTexture().bind(0);
				if (enableShadows) mShadowFBO.getTexture().bind(1);
				mLightFBO.getTexture().bind(2);
				mBlenderShader.bind();
				mBlenderShader.uniform("texSSAO", 0);
				mBlenderShader.uniform("texShadows", 1);
				mBlenderShader.uniform("tex", 2);
				mBlenderShader.uniform("ssao", enableSSAO);
				mBlenderShader.uniform("shadows", enableShadows);
				gl::drawSolidRect(Rectf(0, 0, mFinalFBO.getWidth(), mFinalFBO.getHeight()));
				mBlenderShader.unbind();
				if (enableSSAO) mVBlurFBO.getTexture().unbind(0);
				if (enableShadows) mShadowFBO.getTexture().unbind(1);
				mLightFBO.getTexture().unbind(2);
				mFinalFBO.unbindFramebuffer();

				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize());
				
				mFinalFBO.getTexture().bind(0);
				mFXAAShader.bind();
				mFXAAShader.uniform("tex", 0);
				mFXAAShader.uniform("frameBufSize", Vec2f(mFinalFBO.getWidth(), mFinalFBO.getHeight()));
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mFXAAShader.unbind();
				mFinalFBO.getTexture().unbind(0);
				break;
				
			case SHOW_DIFFUSE_VIEW: // key 1
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mDeferredFBO.getTexture(0).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mDeferredFBO.getTexture(0).unbind(0);
				break;
				
			case SHOW_NORMALMAP_VIEW: // key 2
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mDeferredFBO.getTexture(1).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mDeferredFBO.getTexture(1).unbind(0);
				break;
				
			case SHOW_DEPTH_VIEW: // key 3
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mDeferredFBO.getTexture(1).bind(0);
				mAlphaToRBGShader.bind();
				mAlphaToRBGShader.uniform("tex", 0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mAlphaToRBGShader.unbind();
				mDeferredFBO.getTexture(1).unbind(0);
				break;
				
			case SHOW_POSITION_VIEW: // key 4
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mDeferredFBO.getTexture(2).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mDeferredFBO.getTexture(2).unbind(0);
				break;
				
			case SHOW_ATTRIBUTE_VIEW: // key 5
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mDeferredFBO.getTexture(3).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mDeferredFBO.getTexture(3).unbind(0);
				break;
				
			case SHOW_SSAO_VIEW: // key 6
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mSSAOFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mSSAOFBO.getTexture().unbind(0);
				break;
				
			case SHOW_SSAO_BLURRED_VIEW: // key 7
				renderPingPongBlurToFBO();
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mVBlurFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mVBlurFBO.getTexture().unbind(0);
				break;
				
			case SHOW_LIGHT_VIEW: // key 8
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mLightFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mLightFBO.getTexture().unbind(0);
				break;
				
			case SHOW_SHADOWS_VIEW: // key 9
				gl::setViewport(getWindowBounds());
				gl::setMatricesWindow(getWindowSize(), false);
				mShadowFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mShadowFBO.getTexture().unbind(0);
				break;
		}
	}

	std::vector<PointLight*>& getLights() {
		return mLights;
	};
	
	void addLight(const Vec3f position, const Color color, const bool castsShadows = false, const bool visible = true) {
		mLights.push_back(new PointLight(position, color, PointLight::LIGHT_BRIGHTNESS_DEFAULT, castsShadows, visible));
	}
	
private:
	void renderSceneToDeferredFBO() {
		mDeferredFBO.bindFramebuffer();
		gl::setViewport(mDeferredFBO.getBounds());
		gl::setMatrices(mCamera->getCamera());
		
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mDeferredShader.bind();

		// render deferred light geometry
		mDeferredShader.uniform("diff_coeff", 0.15f);
		mDeferredShader.uniform("phong_coeff", 0.3f);
		mDeferredShader.uniform("two_sided", 0.8f);
		mDeferredShader.uniform("useTexture", 0.0f);
		renderLightGeometry();
		
		// render deferred geometry
		// original values:
		mDeferredShader.uniform("diff_coeff", 1.0f);
		mDeferredShader.uniform("phong_coeff", 0.0f);
		mDeferredShader.uniform("two_sided", 0.0f);
		//mDeferredShader.uniform("diff_coeff", 0.6f);
		//mDeferredShader.uniform("phong_coeff", 0.3f);
		//mDeferredShader.uniform("two_sided", 0.0f);
		if (mRenderShadowCastersFunc) mRenderShadowCastersFunc(&mDeferredShader);
		if (mRenderNonShadowCastersFunc) mRenderNonShadowCastersFunc(&mDeferredShader);
		
		mDeferredShader.unbind();
		mDeferredFBO.unbindFramebuffer();
	}

	void renderShadowsToFBOs() {
		if (!mRenderShadowCastersFunc)
			return;

		// clear final fbo
		mShadowFBO.bindFramebuffer();
		glClearColor(0.5f, 0.5f, 0.5f, 0.0);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mShadowFBO.unbindFramebuffer();

		for (auto light : mLights) {
			if (!light->isShadowCaster())
				continue;
			
			glEnable(GL_CULL_FACE);

			// 1. create shadow map into cube fbo
			
			mCubeDepthFBO.bindFramebuffer();
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glViewport(0, 0, mShadowMapResolution, mShadowMapResolution);
			
			glCullFace(GL_FRONT);
			
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(light->getCamera().getProjectionMatrix());
			glMatrixMode(GL_MODELVIEW);
			
			for (size_t i = 0; i < 6; ++i) {
				mCubeShadowMap.bindDepthFramebuffer(i);
				glClear(GL_DEPTH_BUFFER_BIT);
				
				glLoadMatrixf(mLightFaceViewMatrices[i]);
				glMultMatrixf(light->getCamera().getModelViewMatrix());
				mRenderShadowCastersFunc(0);
				mCubeShadowMap.unbindDepthFramebuffer(i);
			}
			mCubeDepthFBO.unbindFramebuffer();

			// 2. render each shadow layer to shadow fbo (via alpha blending)

			glCullFace(GL_BACK); // don't need what we won't see
			gl::enableAlphaBlending();
			
			mShadowFBO.bindFramebuffer();
			gl::setViewport(mShadowFBO.getBounds());

			gl::setMatrices(mCamera->getCamera());
			
			mCubeShadowShader.bind();
			mCubeShadowMap.bind();
			mCubeShadowShader.uniform("shadowMap", 0);
			
			// conversion from world-space to camera-space (required here)
			mCubeShadowShader.uniform("light_position",
									  mCamera->getCamera().getModelViewMatrix().transformPointAffine(light->getCamera().getEyePoint()));
			mCubeShadowShader.uniform("camera_view_matrix_inv", mCamera->getCamera().getInverseModelViewMatrix());
			mCubeShadowShader.uniform("light_view_matrix", light->getCamera().getModelViewMatrix());
			mCubeShadowShader.uniform("light_projection_matrix", light->getCamera().getProjectionMatrix());
			
			renderLightGeometry();
			if (mRenderShadowCastersFunc) { mRenderShadowCastersFunc(nullptr); }
			if (mRenderNonShadowCastersFunc) { mRenderNonShadowCastersFunc(nullptr); }
			
			mCubeShadowMap.unbind();
			mCubeShadowShader.unbind();
			
			mShadowFBO.unbindFramebuffer();

			gl::disableAlphaBlending();
			glDisable(GL_CULL_FACE);
		}
	}
	
	void renderLightsToFBO() {
		mLightFBO.bindFramebuffer();
		gl::setViewport(mLightFBO.getBounds());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		mLightShader.bind(); //bind point light pixel shader
		mDeferredFBO.getTexture(0).bind(0); // bind color tex
		mDeferredFBO.getTexture(1).bind(1); // bind normal tex
		mDeferredFBO.getTexture(2).bind(2); // bind position, normal and color textures from deferred shading pass
		mDeferredFBO.getTexture(3).bind(3); // bind attr tex
		mLightShader.uniform("colorMap", 0);
		mLightShader.uniform("normalMap", 1);
		mLightShader.uniform("positionMap", 2);
		mLightShader.uniform("attributeMap", 3);
		
		mLightShader.uniform("cameraPosition", mCamera->getCamera().getEyePoint());
		
		// render proxy shapes
		for (PointLight* light : mLights) {
			float radius = light->getRadius();
			mLightShader.uniform("lightPosition", mCamera->getCamera().getModelViewMatrix().transformPointAffine(light->getPosition()));
			mLightShader.uniform("lightColor", light->getColor() * light->getBrightness());
			mLightShader.uniform("dist", radius);
			gl::drawCube(light->getPosition(), Vec3f(radius, radius, radius));
		}
		
		mLightShader.unbind();
		mDeferredFBO.getTexture(0).unbind(0);
		mDeferredFBO.getTexture(1).unbind(1);
		mDeferredFBO.getTexture(2).unbind(2);
		mDeferredFBO.getTexture(3).unbind(3);
		
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		mLightFBO.unbindFramebuffer();
	}
	
	void renderSSAOToFBO() {
		//render out main scene to FBO
		mSSAOFBO.bindFramebuffer();
		gl::setViewport(mSSAOFBO.getBounds());
		gl::setMatricesWindow((float)mSSAOFBO.getWidth(), (float)mSSAOFBO.getHeight());
		
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mDeferredFBO.getTexture(1).bind(0);
		mRandomNoise.bind(1);
		mSSAOShader.bind();
		mSSAOShader.uniform("normalMap", 0);
		mSSAOShader.uniform("randomNoise", 1);
		
		gl::drawSolidRect(Rectf(0, 0, mSSAOFBO.getWidth(), mSSAOFBO.getHeight()));
		
		mSSAOShader.unbind();
		
		mDeferredFBO.getTexture(1).unbind(0);
		mRandomNoise.unbind(1);
		
		mSSAOFBO.unbindFramebuffer();
	}

	void renderPingPongBlurToFBO() {
		//--------- render horizontal blur first --------------
		mHBlurFBO.bindFramebuffer();
		gl::setMatricesWindow((float)mHBlurFBO.getWidth(), (float)mHBlurFBO.getHeight());
		gl::setViewport(mHBlurFBO.getBounds());
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mSSAOFBO.getTexture().bind(0);
		mHBlurShader.bind();
		mHBlurShader.uniform("tex", 0);
		mHBlurShader.uniform("blurStep", 1.0f / mHBlurFBO.getWidth());
		gl::drawSolidRect(Rectf(0, 0, mHBlurFBO.getWidth(), mHBlurFBO.getHeight()));
		mHBlurShader.unbind();
		mSSAOFBO.getTexture().unbind(0);
		mHBlurFBO.unbindFramebuffer();
		
		//--------- now render vertical blur --------------
		mVBlurFBO.bindFramebuffer();
		gl::setViewport(mVBlurFBO.getBounds());
		gl::setMatricesWindow((float)mVBlurFBO.getWidth(), (float)mVBlurFBO.getHeight());
		glClearColor(0.5f, 0.5f, 0.5f, 1);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mHBlurFBO.getTexture().bind(0);
		mVBlurShader.bind();
		mVBlurShader.uniform("tex", 0);
		mVBlurShader.uniform("blurStep", 1.0f / mVBlurFBO.getHeight());
		gl::drawSolidRect(Rectf(0, 0, mVBlurFBO.getWidth(), mVBlurFBO.getHeight()));
		mVBlurShader.unbind();
		mHBlurFBO.getTexture().unbind(0);
		mVBlurFBO.unbindFramebuffer();
	}
	
	void renderLightGeometry() {
		for (PointLight* light : mLights) {
			if (light->isVisible()) {
				gl::drawCube(light->getPosition(), Vec3f(2, 2, 2));
			}
		}
	}
};
