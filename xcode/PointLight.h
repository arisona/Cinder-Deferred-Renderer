#pragma once

using namespace ci;

class PointLight {
	const float LIGHT_CUTOFF_DEFAULT = 0.01f;
	const float LIGHT_BRIGHTNESS_DEFAULT = 100.0f;

private:
    Vec3f mPosition;
    Color mColor;
    float mRadius; // AOE = area of effect
    bool mVisible;
    
public:
    CameraPersp mShadowCam;
    CubeShadowMap mShadowMap;
    gl::Fbo mCubeDepthFbo;
    gl::Fbo mShadowFBO;
    
public:
	PointLight(Vec3f position, Color color, int shadowMapRes, bool castShadows = false, bool visible = true) {
        mPosition = position;
		setColor(color);
        mVisible = visible;
        
        //set up fake "light" to grab matrix calculations from
        mShadowCam.setPerspective(90.0f, 1.0f, 1.0f, 100.0f);
        mShadowCam.lookAt(position, Vec3f(position.x, 0, position.z));
        
        if (castShadows) {
			// set up cube map for point shadows
			mShadowMap.setup(shadowMapRes);
			
			// create FBO to hold depth values from cube map
			gl::Fbo::Format formatShadow;
			formatShadow.enableColorBuffer(true, 1);
			formatShadow.enableDepthBuffer(true, true);
			formatShadow.setMinFilter(GL_LINEAR);
			formatShadow.setMagFilter(GL_LINEAR);
			formatShadow.setWrap(GL_CLAMP, GL_CLAMP);
			mCubeDepthFbo = gl::Fbo(shadowMapRes, shadowMapRes, formatShadow);
			
			gl::Fbo::Format format;
			// format.setDepthInternalFormat(GL_DEPTH_COMPONENT32);
			format.setColorInternalFormat(GL_RGBA16F_ARB);
			// format.setSamples(4); // enable 4x antialiasing
			mShadowFBO	= gl::Fbo(shadowMapRes, shadowMapRes, format);
		}
    }
    
	void setPosition(const Vec3f& position) {
        mShadowCam.lookAt(position, Vec3f(position.x, 0.0f, position.z));
        mPosition = position;
    }
    
    const Vec3f& getPosition() const {
        return mPosition;
    }
    
	void setColor(const Color& color) {
        mColor = color;
        mRadius = sqrt(mColor.length() / LIGHT_CUTOFF_DEFAULT * LIGHT_BRIGHTNESS_DEFAULT);
    }
    
    const Color& getColor() const {
        return mColor;
    }
	
	float getBrightness() const {
		return LIGHT_BRIGHTNESS_DEFAULT;
	}
    
    float getRadius() const {
        return mRadius;
    }
    
    bool isVisible() const {
		return mVisible;
	}

    bool isShadowCaster() const {
		return mShadowFBO;
	}
};
