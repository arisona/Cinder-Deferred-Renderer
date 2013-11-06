#version 120

varying vec3 position;
varying vec3 normal;

void main(void) {
	gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    vec4 tmp = gl_ModelViewMatrix * gl_Vertex;
	position = tmp.xyz/tmp.w;
	normal = gl_NormalMatrix * gl_Normal;
}
