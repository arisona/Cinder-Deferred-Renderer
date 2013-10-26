#pragma once

using namespace ci;

class PointLight {
public:
	static constexpr float LIGHT_BRIGHTNESS_DEFAULT = 60.0f;

private:
	static constexpr float LIGHT_CUTOFF_DEFAULT = 0.01f;

    Vec3f mPosition;
    Color mColor;
    float mRadius;
	float mBrightness;
	bool mCastShadows;
    bool mVisible;
    
public:
    CameraPersp mShadowCam;
    
public:
	PointLight(Vec3f position, Color color, float brightness, bool castShadows = false, bool visible = true) {
        mPosition = position;
		setColor(color);
		mBrightness = brightness;
		mCastShadows = castShadows;
        mVisible = visible;
        
        mShadowCam.setPerspective(90, 1, 1, 10000);
        mShadowCam.lookAt(position, Vec3f(position.x, 0, position.z));
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
		return mCastShadows;
	}
};
