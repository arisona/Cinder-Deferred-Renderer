#version 120

varying vec2 uv;

// texture coordinate for screen aligned fragment shader (in correct range):
void main(void) {
	gl_Position = ftransform();
	//gl_Position = sign(gl_Position);
	uv = gl_Position.xy * 0.5 + 0.5;
}
