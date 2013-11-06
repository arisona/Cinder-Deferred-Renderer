#version 120

varying vec3 position; // in view space
varying vec3 normal; // in view space

void main(void) {
	gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
	vec4 p = gl_ModelViewMatrix * gl_Vertex;
    position = p.xyz / p.w;
	normal = gl_NormalMatrix * gl_Normal;
}
