#version 120

// basic shader to change an alpha value to black

uniform sampler2D tex;

varying vec2  uv;

void main(void) {
    vec4 diffuse = texture2D(tex, uv);
    gl_FragColor = vec4(diffuse.a, diffuse.a, diffuse.a, 1.0);
}