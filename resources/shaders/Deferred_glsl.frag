#version 120

uniform sampler2D tex;

uniform float diff_coeff;
uniform float phong_coeff;
uniform float two_sided;
uniform float useTexture;

varying vec3 position;
varying vec3 normal;
varying float depth;

void main(void) {
	// remove fragments with low alpha
	if (gl_Color.a < 0.1) discard;
	vec3 n = normalize(normal);
    
    // get texture if one binded otherwise use color as defined by useTexture float (range: 0.0 - 1.0)
    vec2 uv = gl_TexCoord[0].st;
    uv.t = 1.0 - uv.t;
    vec4 texColor = texture2D(tex, uv) * useTexture;
    vec4 vertColor = gl_Color * (1.0 - useTexture);
    
	gl_FragData[0] = texColor + vertColor;
	gl_FragData[1] = vec4(n, depth);
    gl_FragData[2] = vec4(position, 1.0);
	gl_FragData[3] = vec4(diff_coeff, phong_coeff, two_sided, 1.0);
}
