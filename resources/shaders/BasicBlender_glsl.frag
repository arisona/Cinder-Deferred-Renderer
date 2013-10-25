#version 120

// just blending 3 textures together (in a weird way)


uniform sampler2D texSSAO;
uniform sampler2D texShadows;
uniform sampler2D tex;

uniform bool ssao;
uniform bool shadows;

varying vec2 uv;

void main() {
    // blending by red value (from ssao)
	float red = ssao ? 1.0 - texture2D(texSSAO, uv).r : 0;
  
	// blending by alpha (from shadows)
    float alpha = shadows ? texture2D(texShadows, uv).a : 0;

	vec4 color = texture2D(tex, uv);
	gl_FragColor = vec4(color.r - red - alpha, color.g - red - alpha, color.b - red - alpha, color.a - red - alpha);
}
