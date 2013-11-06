#version 120

uniform sampler2D tex;

uniform float diff_coeff;
uniform float phong_coeff;
uniform float two_sided;
uniform bool useTexture;

varying vec3 position;
varying vec3 normal;

void main(void) {
	// remove fragments with low alpha
	if (gl_Color.a < 0.1) discard;
	vec3 n = normalize(normal);
    
	vec4 color;
	if (useTexture) {
		vec2 uv = gl_TexCoord[0].st;
		color = texture2D(tex, uv);
	} else {
		color = gl_Color;
	}
    
	gl_FragData[0] = color;
	gl_FragData[1] = vec4(n, position.z);
    gl_FragData[2] = vec4(position.xyz, 1.0);
	gl_FragData[3] = vec4(diff_coeff, phong_coeff, two_sided, 1.0);
}
