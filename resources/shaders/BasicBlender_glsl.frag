#version 120

// just blending 3 textures together (in a weird way)

uniform sampler2D ssaoTex;
uniform sampler2D shadowsTex;
uniform sampler2D baseTex;

varying vec2 uv;

void main() {
	vec4 ssao = texture2D(ssaoTex, uv);
    vec4 shadow	= texture2D(shadowsTex, uv);
	vec4 base = texture2D(baseTex, uv);
    
    // blending by red value (from ssao)
	float red = 1.0 - ssao.r;
  
	// blending by alpha (from shadows)
    float alpha = shadow.a;

	gl_FragColor = vec4(base.r - red - alpha, base.g - red - alpha, base.b - red - alpha, base.a - red - alpha);
}
