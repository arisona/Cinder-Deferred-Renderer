#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

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
    
	unsigned int textureObject;
    
public:
    CubeShadowMap() {}
    
	void setup(GLsizei texSize) {
        glGenTextures(1, &textureObject);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    
	void bind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);
    }
    
    void bindDepthFramebuffer(const int face) const {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureObject, 0);
    }

	void unbind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};
