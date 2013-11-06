#version 120

// utility shader to display alpha values (depth buffer in this case)

uniform sampler2D tex;

varying vec2 uv;

void main(void) {
    float alpha = texture2D(tex, uv).a / -100;
    gl_FragColor = vec4(alpha, alpha, alpha, 1.0);
}
