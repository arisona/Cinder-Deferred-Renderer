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

	Matrix44f mLightFaceViewMatrices[6];

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
        
        // init shadow cube: axial matrices required for six-sides of calculations for cube shadows
        CameraPersp cubeCam;
        cubeCam.lookAt(Vec3f(0, 0, 0),  Vec3f(1, 0, 0),  Vec3f(0, -1, 0));
        mLightFaceViewMatrices[CubeShadowMap::X_FACE_POS] = cubeCam.getModelViewMatrix();
		
        cubeCam.lookAt(Vec3f(0, 0, 0), Vec3f(-1, 0, 0),  Vec3f(0, -1, 0));
        mLightFaceViewMatrices[CubeShadowMap::X_FACE_NEG] = cubeCam.getModelViewMatrix();
		
        cubeCam.lookAt(Vec3f(0, 0, 0),  Vec3f(0, 1, 0), Vec3f(0, 0, 1));
        mLightFaceViewMatrices[CubeShadowMap::Y_FACE_POS] = cubeCam.getModelViewMatrix();
		
        cubeCam.lookAt(Vec3f(0, 0, 0),  Vec3f(0.0,-1.0, 0.0),  Vec3f(0.0, 0.0,-1.0));
        mLightFaceViewMatrices[CubeShadowMap::Y_FACE_NEG] = cubeCam.getModelViewMatrix();
		
        cubeCam.lookAt(Vec3f(0, 0, 0),  Vec3f(0, 0, 1),  Vec3f(0, -1, 0));
        mLightFaceViewMatrices[CubeShadowMap::Z_FACE_POS] = cubeCam.getModelViewMatrix();
		
        cubeCam.lookAt(Vec3f(0, 0, 0),  Vec3f(0, 0, -1),  Vec3f(0, -1, 0));
        mLightFaceViewMatrices[CubeShadowMap::Z_FACE_NEG] = cubeCam.getModelViewMatrix();

        // init textures
		mRandomNoise = gl::Texture(loadImage(loadResource(NOISE_SAMPLER))); //noise texture required for SSAO calculations
		
		// init shaders
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
		
		// init FBOs
		//this FBO will capture normals, depth, and base diffuse in one render pass (as opposed to three)
		gl::Fbo::Format mtRFBO;
		mtRFBO.enableDepthBuffer();
		mtRFBO.setDepthInternalFormat(GL_DEPTH_COMPONENT32); //want fbo to have precision depth map as well
		mtRFBO.setColorInternalFormat(GL_RGBA16F_ARB);
		mtRFBO.enableColorBuffer(true, 4); // create an FBO with four color attachments (basic diffuse, normal/depth view, attribute view, and position view)
		//mtRFBO.setSamples(4); // uncomment this to enable 4x antialiasing
		
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
        
		// 2. create shadow maps, and render shadows to FBOs
		
        createShadowMaps();
        renderShadowsToFBOs();
        
        gl::setMatrices(mCamera->getCamera());
        
        renderLightsToFBO();
        renderSSAOToFBO();
		
        // 3. final deferred rendering, depending on current mode
		
        switch (renderMode) {
            case SHOW_FINAL_VIEW:
                pingPongBlurSSAO();
				
				mFinalFBO.bindFramebuffer();
				glClearColor(0.5f, 0.5f, 0.5f, 1);
				glClearDepth(1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                gl::setMatricesWindow((float)mFinalFBO.getWidth(), (float)mFinalFBO.getHeight());
                gl::setViewport(mFinalFBO.getBounds());
                mVBlurFBO.getTexture().bind(0);
                mShadowFBO.getTexture().bind(1);
                mLightFBO.getTexture().bind(2);
                mBlenderShader.bind();
                mBlenderShader.uniform("ssaoTex", 0);
                mBlenderShader.uniform("shadowsTex", 1);
                mBlenderShader.uniform("baseTex", 2);
                gl::drawSolidRect(Rectf(0, 0, mFinalFBO.getWidth(), mFinalFBO.getHeight()));
                mBlenderShader.unbind();
                mLightFBO.getTexture().unbind(2);
                mShadowFBO.getTexture().unbind(1);
                mVBlurFBO.getTexture().unbind(0);
                
				mFinalFBO.unbindFramebuffer();
                
				mFinalFBO.getTexture().bind(0);
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize());
				mFXAAShader.bind();
				mFXAAShader.uniform("tex", 0);
				mFXAAShader.uniform("frameBufSize", Vec2f(mFinalFBO.getWidth(), mFinalFBO.getHeight()));
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mFXAAShader.unbind();
				mFinalFBO.getTexture().unbind(0);
                break;
				
            case SHOW_DIFFUSE_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mDeferredFBO.getTexture(0).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(0).unbind(0);
                break;
				
            case SHOW_DEPTH_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mDeferredFBO.getTexture(1).bind(0);
                mAlphaToRBGShader.bind();
                mAlphaToRBGShader.uniform("tex", 0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mAlphaToRBGShader.unbind();
                mDeferredFBO.getTexture(1).unbind(0);
                break;
				
            case SHOW_POSITION_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mDeferredFBO.getTexture(2).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(2).unbind(0);
                break;
				
            case SHOW_ATTRIBUTE_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mDeferredFBO.getTexture(3).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(3).unbind(0);
                break;
                
            case SHOW_NORMALMAP_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mDeferredFBO.getTexture(1).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(1).unbind(0);
                break;
				
            case SHOW_SSAO_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mSSAOFBO.getTexture().bind(0);
                mBlenderShader.bind();
                mBlenderShader.uniform("ssaoTex", 0);
                mBlenderShader.uniform("shadowsTex", 0);
                mBlenderShader.uniform("baseTex", 0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mBlenderShader.unbind();
                mSSAOFBO.getTexture().unbind(0);
                break;
				
            case SHOW_SSAO_BLURRED_VIEW:
                pingPongBlurSSAO();
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mVBlurFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mVBlurFBO.getTexture().unbind(0);
                break;
				
            case SHOW_LIGHT_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize(), false);
                mLightFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mLightFBO.getTexture().unbind(0);
                break;
				
            case SHOW_SHADOWS_VIEW:
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
        mLights.push_back(new PointLight(position, color, mShadowMapResolution, castsShadows, visible));
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
		mDeferredShader.uniform("diff_coeff", 1.0f);
		mDeferredShader.uniform("phong_coeff", 0.0f);
		mDeferredShader.uniform("two_sided", 0.8f);
		if (mRenderShadowCastersFunc) mRenderShadowCastersFunc(&mDeferredShader);
		if (mRenderNonShadowCastersFunc) mRenderNonShadowCastersFunc(&mDeferredShader);
		
		mDeferredShader.unbind();
		mDeferredFBO.unbindFramebuffer();
	}

    void createShadowMaps() {
        //render depth map cube
		if (!mRenderShadowCastersFunc)
			return
        glEnable(GL_CULL_FACE);
		for (PointLight* light : mLights) {
            if (!light->isShadowCaster())
				continue;
            
            light->mCubeDepthFbo.bindFramebuffer();
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glViewport(0, 0, mShadowMapResolution, mShadowMapResolution);
            
            glCullFace(GL_FRONT);
            
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(light->mShadowCam.getProjectionMatrix());
            glMatrixMode(GL_MODELVIEW);
            
            for (size_t i = 0; i < 6; ++i) {
                light->mShadowMap.bindDepthFramebuffer(i);
                glClear(GL_DEPTH_BUFFER_BIT);
                
                glLoadMatrixf(mLightFaceViewMatrices[i]);
                glMultMatrixf(light->mShadowCam.getModelViewMatrix());
					mRenderShadowCastersFunc(0);
            }
            
            light->mCubeDepthFbo.unbindFramebuffer();
		}
        glDisable(GL_CULL_FACE);
    }
    
    void renderShadowsToFBOs() {
        glEnable(GL_CULL_FACE);
		for (PointLight* light : mLights) {
            if (!light->isShadowCaster())
				continue;
            
            light->mShadowFBO.bindFramebuffer();
            
            glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            //glDrawBuffer(GL_BACK);
            //glReadBuffer(GL_BACK);
            gl::setViewport(light->mShadowFBO.getBounds());
            
            glCullFace(GL_BACK); //don't need what we won't see
            
            gl::setMatrices(mCamera->getCamera());
            
            mCubeShadowShader.bind();
			
            light->mShadowMap.bind(); //the magic texture
            mCubeShadowShader.uniform("shadow", 0);
            
			//conversion from world-space to camera-space (required here)
            mCubeShadowShader.uniform("light_position", mCamera->getCamera().getModelViewMatrix().transformPointAffine(light->mShadowCam.getEyePoint()));
            mCubeShadowShader.uniform("camera_view_matrix_inv", mCamera->getCamera().getInverseModelViewMatrix());
            mCubeShadowShader.uniform("light_view_matrix", light->mShadowCam.getModelViewMatrix());
            mCubeShadowShader.uniform("light_projection_matrix", light->mShadowCam.getProjectionMatrix());
            
			renderLightGeometry();
			if (mRenderShadowCastersFunc) { mRenderShadowCastersFunc(nullptr); }
			if (mRenderNonShadowCastersFunc) { mRenderNonShadowCastersFunc(nullptr); }
            
            light->mShadowMap.unbind();
            glDisable(GL_TEXTURE_CUBE_MAP);
            
            mCubeShadowShader.unbind();
            
            light->mShadowFBO.unbindFramebuffer();
        }
        glDisable(GL_CULL_FACE);
        
        //render all shadow layers to one FBO
        mShadowFBO.bindFramebuffer();
        gl::setViewport(mShadowFBO.getBounds());
        gl::setMatricesWindow((float)mShadowFBO.getWidth(), (float)mShadowFBO.getHeight());
        glClearColor(0.5f, 0.5f, 0.5f, 0.0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl::enableAlphaBlending();
        
		for (PointLight* light : mLights) {
            if (!light->isShadowCaster())
				continue;
            
            light->mShadowFBO.getTexture().bind();
			//this is different as we are not using shaders to color these quads (need to fit viewport)
            gl::drawSolidRect(Rectf(0, (float)mShadowFBO.getHeight(), (float)mShadowFBO.getWidth(), 0));
            light->mShadowFBO.getTexture().unbind();
        }
        gl::disableAlphaBlending();
        mShadowFBO.unbindFramebuffer();
    }
    
    void renderSSAOToFBO() {
        //render out main scene to FBO
        mSSAOFBO.bindFramebuffer();
        gl::setViewport(mSSAOFBO.getBounds());
        gl::setMatricesWindow((float)mSSAOFBO.getWidth(), (float)mSSAOFBO.getHeight());
        
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mRandomNoise.bind(0);
        mDeferredFBO.getTexture(1).bind(1);
        mSSAOShader.bind();
        mSSAOShader.uniform("rnm", 0);
        mSSAOShader.uniform("normalMap", 1);
        
        gl::drawSolidRect(Rectf(0, 0, mSSAOFBO.getWidth(), mSSAOFBO.getHeight()));
        
        mSSAOShader.unbind();
        
        mDeferredFBO.getTexture(1).unbind(1);
        mRandomNoise.unbind(0);
        
        mSSAOFBO.unbindFramebuffer();
    }
    
    void renderLightsToFBO() {
        mLightFBO.bindFramebuffer();
        gl::setViewport(mLightFBO.getBounds());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //draw glowing cubes
        renderLights();
        mLightFBO.unbindFramebuffer();
    }
    
    void pingPongBlurSSAO() {
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
    
    void renderLights() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE); //set blend function
        glEnable(GL_CULL_FACE); //cull front faces
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST); //disable depth testing
        glDepthMask(false);
        
        mLightShader.bind(); //bind point light pixel shader
        mDeferredFBO.getTexture(2).bind(0); //bind position, normal and color textures from deferred shading pass
        mLightShader.uniform("positionMap", 0);
        mDeferredFBO.getTexture(1).bind(1); //bind normal tex
        mLightShader.uniform("normalMap", 1);
        mDeferredFBO.getTexture(0).bind(2); //bind color tex
        mLightShader.uniform("colorMap", 2);
        mDeferredFBO.getTexture(3).bind(3); //bind attr tex
        mLightShader.uniform("attrMap", 3);
        mLightShader.uniform("camPosition", mCamera->getCamera().getEyePoint());
        
		// render  proxy shapes
        for (PointLight* light : mLights) {
			float distance = light->getAOEDistance();
			mLightShader.uniform("lightPos", mCamera->getCamera().getModelViewMatrix().transformPointAffine(light->getPosition()));
			mLightShader.uniform("lightCol", light->getColor());
			mLightShader.uniform("dist", distance);
			gl::drawCube(light->getPosition(), Vec3f(distance, distance, distance));
        }
        
        mLightShader.unbind();
        mDeferredFBO.getTexture(2).unbind(0);
        mDeferredFBO.getTexture(1).unbind(1);
        mDeferredFBO.getTexture(0).unbind(2);
        mDeferredFBO.getTexture(3).unbind(3);
        
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDisable(GL_BLEND);
    }
	
	void renderLightGeometry() {
		for (PointLight* light : mLights) {
			if (light->isVisible()) {
				gl::drawCube(light->getPosition(), Vec3f(2, 2, 2));
			}
		}
	}
};
