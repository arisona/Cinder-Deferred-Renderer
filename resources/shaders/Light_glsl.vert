#version 120

varying vec4 pos;

void main(void) {
	vec4 p = ftransform();
	gl_Position = p;
	pos = p;
}