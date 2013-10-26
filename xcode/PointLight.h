#pragma once

using namespace ci;

class PointLight {
public:
	static constexpr float LIGHT_BRIGHTNESS_DEFAULT = 60.0f;

private:
	static constexpr float LIGHT_CUTOFF_DEFAULT = 0.01f;

    Vec3f mPosition;
    Color mColor;
    float mRadius; // AOE = area of effect
	float mBrightness;
    bool mVisible;
    
public:
    CameraPersp mShadowCam;
    CubeShadowMap mCubeShadowMap;
    gl::Fbo mCubeDepthFbo;
    gl::Fbo mShadowFBO;
    
public:
	PointLight(Vec3f position, Color color, float brightness, int shadowMapRes, bool castShadows = false, bool visible = true) {
        mPosition = position;
		setColor(color);
		mBrightness = brightness;
        mVisible = visible;
        
        mShadowCam.setPerspective(90, 1, 1, 10000);
        mShadowCam.lookAt(position, Vec3f(position.x, 0, position.z));
        
        if (castShadows) {
			// set up cube map for point shadows
			mCubeShadowMap.setup(shadowMapRes);
			
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
    }
    
    const Color& getColor() const {
        return mColor;
    }
	
	void setBrightness(float brightness) {
		mBrightness = brightness;
	}
	
	float getBrightness() const {
		return mBrightness;
	}
    
    float getRadius() const {
        return sqrt(mColor.length() / LIGHT_CUTOFF_DEFAULT * mBrightness);
    }
    
    bool isVisible() const {
		return mVisible;
	}

    bool isShadowCaster() const {
		return mShadowFBO;
	}
};
