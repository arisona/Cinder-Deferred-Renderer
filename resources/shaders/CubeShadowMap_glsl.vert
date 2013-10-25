#version 120
#extension GL_EXT_gpu_shader4 : require

varying vec4 position;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	position = gl_ModelViewMatrix * gl_Vertex;
}
