#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

// XXX destructure/dispose missing...
class CubeShadowMap {
public:
    enum {
        X_FACE_POS,
        X_FACE_NEG,
        Y_FACE_POS,
        Y_FACE_NEG,
        Z_FACE_POS,
        Z_FACE_NEG
    };

private:
	unsigned int mTexture = 0;
    
public:
    CubeShadowMap() {}
    
	void setup(GLsizei mTextureSize) {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, mTextureSize, mTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    
	void bind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    }
    
    void bindDepthFramebuffer(int face) const {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mTexture, 0);
    }
	
	void unbindDepthFramebuffer(int face) const {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0);
	}

	void unbind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};
