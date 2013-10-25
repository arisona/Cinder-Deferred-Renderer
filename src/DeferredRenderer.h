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
    
    Matrix44f mLightFaceViewMatrices[6];
	
    gl::Texture mRandomNoise;
    
    gl::Fbo mDeferredFBO;
    gl::Fbo mSSAOMap;
    gl::Fbo	mPingPongBlurH;
    gl::Fbo mPingPongBlurV;
    gl::Fbo mLightGlowFBO;
    gl::Fbo mAllShadowsFBO;
	gl::Fbo mFinalSSFBO;
    
	gl::GlslProg mCubeShadowShader;
	
    gl::GlslProg mSSAOShader;
    gl::GlslProg mDeferredShader;
    gl::GlslProg mBasicBlender;
    gl::GlslProg mHBlurShader;
    gl::GlslProg mVBlurShader;
    gl::GlslProg mLightShader;
    gl::GlslProg mAlphaToRBG;
	gl::GlslProg mFXAAShader;
    
    std::vector<PointLight*> mLights;
    
    Vec2i mFBOResolution;
    int mShadowMapResolution;
    
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
		mSSAOShader			= gl::GlslProg(loadResource(SSAO_VERT), loadResource(SSAO_FRAG));
		mDeferredShader		= gl::GlslProg(loadResource(DEFER_VERT), loadResource(DEFER_FRAG));
		mBasicBlender		= gl::GlslProg(loadResource(BBlender_VERT), loadResource(BBlender_FRAG));
		mHBlurShader		= gl::GlslProg(loadResource(BLUR_H_VERT), loadResource(BLUR_H_FRAG));
		mVBlurShader		= gl::GlslProg(loadResource(BLUR_V_VERT), loadResource(BLUR_V_FRAG));
		mLightShader		= gl::GlslProg(loadResource(LIGHT_VERT), loadResource(LIGHT_FRAG));
		mAlphaToRBG         = gl::GlslProg(loadResource(ALPHA_RGB_VERT), loadResource(ALPHA_RGB_FRAG));
		mCubeShadowShader   = gl::GlslProg(loadResource(RES_SHADER_CUBESHADOW_VERT), loadResource(RES_SHADER_CUBESHADOW_FRAG));
		mFXAAShader			= gl::GlslProg(loadResource(RES_SHADER_FXAA_VERT), loadResource(RES_SHADER_FXAA_FRAG));
		
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
		
		//init screen space render
		mDeferredFBO	= gl::Fbo(mFBOResolution.x, mFBOResolution.y, mtRFBO);
		mLightGlowFBO   = gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		mPingPongBlurH	= gl::Fbo(mFBOResolution.x/2, mFBOResolution.y/2, format); //don't need as high res on ssao as it will be blurred anyhow ...
		mPingPongBlurV	= gl::Fbo(mFBOResolution.x/2, mFBOResolution.y/2, format);
		mSSAOMap		= gl::Fbo(mFBOResolution.x/2, mFBOResolution.y/2, format);
		mAllShadowsFBO  = gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		mFinalSSFBO		= gl::Fbo(mFBOResolution.x, mFBOResolution.y, format);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    
    void render(RenderMode renderMode) {
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
		
        // deferred rendering, depending on current mode
		
        switch (renderMode) {
            case SHOW_FINAL_VIEW:
                pingPongBlurSSAO();
				
				mFinalSSFBO.bindFramebuffer();
				glClearColor(0.5f, 0.5f, 0.5f, 1);
				glClearDepth(1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                gl::setMatricesWindow((float)mFinalSSFBO.getWidth(), (float)mFinalSSFBO.getHeight());
                gl::setViewport(mFinalSSFBO.getBounds());
                mPingPongBlurV.getTexture().bind(0);
                mAllShadowsFBO.getTexture().bind(1);
                mLightGlowFBO.getTexture().bind(2);
                mBasicBlender.bind();
                mBasicBlender.uniform("ssaoTex", 0);
                mBasicBlender.uniform("shadowsTex", 1);
                mBasicBlender.uniform("baseTex", 2);
                gl::drawSolidRect(Rectf(0, 0, mFinalSSFBO.getWidth(), mFinalSSFBO.getHeight()));
                mBasicBlender.unbind();
                mLightGlowFBO.getTexture().unbind(2);
                mAllShadowsFBO.getTexture().unbind(1);
                mPingPongBlurV.getTexture().unbind(0);
                
				mFinalSSFBO.unbindFramebuffer();
                
				mFinalSSFBO.getTexture().bind(0);
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
				mFXAAShader.bind();
				mFXAAShader.uniform("buf0", 0);
				mFXAAShader.uniform("frameBufSize", Vec2f(mFinalSSFBO.getWidth(), mFinalSSFBO.getHeight()));
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
				mFXAAShader.unbind();
				mFinalSSFBO.getTexture().unbind(0);
                break;
				
            case SHOW_DIFFUSE_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mDeferredFBO.getTexture(0).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(0).unbind(0);
                break;
				
            case SHOW_DEPTH_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mDeferredFBO.getTexture(1).bind(0);
                mAlphaToRBG.bind();
                mAlphaToRBG.uniform("alphaTex", 0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mAlphaToRBG.unbind();
                mDeferredFBO.getTexture(1).unbind(0);
                break;
				
            case SHOW_POSITION_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mDeferredFBO.getTexture(2).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(2).unbind(0);
                break;
				
            case SHOW_ATTRIBUTE_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mDeferredFBO.getTexture(3).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(3).unbind(0);
                break;
                
            case SHOW_NORMALMAP_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mDeferredFBO.getTexture(1).bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mDeferredFBO.getTexture(1).unbind(0);
                break;
				
            case SHOW_SSAO_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize()); //want textures to fill screen
                mSSAOMap.getTexture().bind(0);
                mBasicBlender.bind();
                mBasicBlender.uniform("ssaoTex", 0);
                mBasicBlender.uniform("shadowsTex", 0);
                mBasicBlender.uniform("baseTex", 0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mBasicBlender.unbind();
                mSSAOMap.getTexture().unbind(0);
                break;
				
            case SHOW_SSAO_BLURRED_VIEW:
                pingPongBlurSSAO();
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize());
                mPingPongBlurV.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mPingPongBlurV.getTexture().unbind(0);
                break;
				
            case SHOW_LIGHT_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize());
                mLightGlowFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mLightGlowFBO.getTexture().unbind(0);
                break;
				
            case SHOW_SHADOWS_VIEW:
                gl::setViewport(getWindowBounds());
                gl::setMatricesWindow(getWindowSize());
                mAllShadowsFBO.getTexture().bind(0);
				gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight()));
                mAllShadowsFBO.getTexture().unbind(0);
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
		
		// render deferred light geometry
		mDeferredShader.bind();
		mDeferredShader.uniform("diff_coeff", 0.15f);
		mDeferredShader.uniform("phong_coeff", 0.3f);
		mDeferredShader.uniform("two_sided", 0.8f);
		mDeferredShader.uniform("useTexture", 0.0f);
		renderLightGeometry();
		
		// render deferred geometry
		//mDeferredShader.bind();
		mDeferredShader.uniform("diff_coeff", 1.0f);
		mDeferredShader.uniform("phong_coeff", 0.0f);
		mDeferredShader.uniform("two_sided", 0.8f);
		
		if (mRenderShadowCastersFunc)
			mRenderShadowCastersFunc(&mDeferredShader);
		
		if (mRenderNonShadowCastersFunc)
			mRenderNonShadowCastersFunc(&mDeferredShader);
		
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
            
            light->mShadowsFbo.bindFramebuffer();
            
            glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            //glDrawBuffer(GL_BACK);
            //glReadBuffer(GL_BACK);
            gl::setViewport(light->mShadowsFbo.getBounds());
            
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
            
            light->mShadowsFbo.unbindFramebuffer();
        }
        glDisable(GL_CULL_FACE);
        
        //render all shadow layers to one FBO
        mAllShadowsFBO.bindFramebuffer();
        gl::setViewport(mAllShadowsFBO.getBounds());
        gl::setMatricesWindow((float)mAllShadowsFBO.getWidth(), (float)mAllShadowsFBO.getHeight()); //want textures to fill FBO
        glClearColor(0.5f, 0.5f, 0.5f, 0.0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl::enableAlphaBlending();
        
		for (PointLight* light : mLights) {
            if (!light->isShadowCaster())
				continue;
            
            light->mShadowsFbo.getTexture().bind();
			//this is different as we are not using shaders to color these quads (need to fit viewport)
            gl::drawSolidRect(Rectf(0, (float)mAllShadowsFBO.getHeight(), (float)mAllShadowsFBO.getWidth(), 0));
            light->mShadowsFbo.getTexture().unbind();
        }
        gl::disableAlphaBlending();
        mAllShadowsFBO.unbindFramebuffer();
    }
    
    void renderSSAOToFBO() {
        //render out main scene to FBO
        mSSAOMap.bindFramebuffer();
        gl::setViewport(mSSAOMap.getBounds());
        gl::setMatricesWindow((float)mSSAOMap.getWidth(), (float)mSSAOMap.getHeight()); //setting orthogonal view as rendering to a fullscreen quad
        
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mRandomNoise.bind(0);
        mDeferredFBO.getTexture(1).bind(1);
        mSSAOShader.bind();
        mSSAOShader.uniform("rnm", 0);
        mSSAOShader.uniform("normalMap", 1);
        
        gl::drawSolidRect(Rectf(0, 0, mSSAOMap.getWidth(), mSSAOMap.getHeight()));
        
        mSSAOShader.unbind();
        
        mDeferredFBO.getTexture(1).unbind(1);
        mRandomNoise.unbind(0);
        
        mSSAOMap.unbindFramebuffer();
    }
    
    void renderLightsToFBO() {
        mLightGlowFBO.bindFramebuffer();
        gl::setViewport(mLightGlowFBO.getBounds());
        glClearColor(0.0f, 0.0f, 0.0f, 1.0);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //draw glowing cubes
        renderLights();
        mLightGlowFBO.unbindFramebuffer();
    }
    
    void pingPongBlurSSAO() {
        //--------- render horizontal blur first --------------
        mPingPongBlurH.bindFramebuffer();
        gl::setMatricesWindow((float)mPingPongBlurH.getWidth(), (float)mPingPongBlurH.getHeight());
        gl::setViewport(mPingPongBlurH.getBounds());
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mSSAOMap.getTexture().bind(0);
        mHBlurShader.bind();
        mHBlurShader.uniform("RTScene", 0);
        mHBlurShader.uniform("blurStep", 1.0f / mPingPongBlurH.getWidth()); //so that every "blur step" will equal one pixel width of this FBO
        gl::drawSolidRect(Rectf(0, 0, mPingPongBlurH.getWidth(), mPingPongBlurH.getHeight()));
        mHBlurShader.unbind();
        mSSAOMap.getTexture().unbind(0);
        mPingPongBlurH.unbindFramebuffer();
        
        //--------- now render vertical blur --------------
        mPingPongBlurV.bindFramebuffer();
        gl::setViewport(mPingPongBlurV.getBounds());
        gl::setMatricesWindow((float)mPingPongBlurV.getWidth(), (float)mPingPongBlurV.getHeight());
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mPingPongBlurH.getTexture().bind(0);
        mHBlurShader.bind();
        mHBlurShader.uniform("RTBlurH", 0);
        mHBlurShader.uniform("blurStep", 1.0f / mPingPongBlurH.getHeight()); //so that every "blur step" will equal one pixel height of this FBO
        gl::drawSolidRect(Rectf(0, 0, mPingPongBlurH.getWidth(), mPingPongBlurH.getHeight()));
        mHBlurShader.unbind();
        mPingPongBlurH.getTexture().unbind(0);
        mPingPongBlurV.unbindFramebuffer();
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
