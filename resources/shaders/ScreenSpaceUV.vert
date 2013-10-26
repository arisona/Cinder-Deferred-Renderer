#version 120

varying vec2 uv;

void main(void) {
	gl_Position = ftransform();
	//gl_Position = sign(gl_Position);
    //gl_FrontColor = gl_Color;
	
	// texture coordinate for screen aligned (in correct range):
	uv = gl_Position.xy * 0.5 + 0.5;
}
