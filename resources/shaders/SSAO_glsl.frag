#version 120
// http://www.gamerendering.com/2009/01/14/ssao/

uniform sampler2D normalMap;
uniform sampler2D randomNoise;

varying vec2 uv;

// may have to change these as they are generally scene-dependent (to get the look you want)
const float totStrength = 0.88;
const float strength = 0.6;
const float offset = 0.02;
const float falloff = 0.01;
const float rad = 0.02;

const float invSamples = 1 / 10.0;

// NOTE: THIS ONE IS BRUTALLY OPTIMIZED!! SO IT*S REALLY HARD TO FOLLOW

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void) {
    // these are the random vectors inside a unit sphere
    // vec3 sphere[10] = vec3[](vec3(-0.010735935, 0.01647018, 0.0062425877),vec3(-0.06533369, 0.3647007, -0.13746321),vec3(-0.6539235, -0.016726388, -0.53000957),vec3(0.40958285, 0.0052428036, -0.5591124),vec3(-0.1465366, 0.09899267, 0.15571679),vec3(-0.44122112, -0.5458797, 0.04912532),vec3(0.03755566, -0.10961345, -0.33040273),vec3(0.019100213, 0.29652783, 0.066237666),vec3(0.8765323, 0.011236004, 0.28265962),vec3(0.29264435, -0.40794238, 0.15964167));

    // these are the random vectors inside a unit sphere
	// NOTE: must be defined this way in GLSL 1.2
    vec3 sphere[10];
    sphere[0] = vec3(0.13790712, 0.24864247, 0.44301823);
    sphere[1] = vec3(0.33715037, 0.56794053, -0.005789503);
    sphere[2] = vec3(0.06896307, -0.15983082, -0.85477847);
    sphere[3] = vec3(-0.014653638, 0.14027752, 0.0762037);
    sphere[4] = vec3(0.010019933, -0.1924225, -0.034443386);
    sphere[5] = vec3(-0.35775623, -0.5301969, -0.43581226);
    sphere[6] = vec3(-0.3169221, 0.106360726, 0.015860917);
    sphere[7] = vec3(0.010350345, -0.58698344, 0.0046293875);
    sphere[8] = vec3(-0.053382345, 0.059675813, -0.5411899);
    sphere[9] = vec3(0.035267662, -0.063188605, 0.54602677);

    // grab a normal for reflecting the sample rays later on
    vec3 fres = normalize((texture2D(randomNoise, rand(uv) * offset * uv).xyz * 2.0) - vec3(1.0));

    vec4 currentPixelSample = texture2D(normalMap, uv);

    float currentPixelDepth = currentPixelSample.a;

    // current fragment coords in screen space
    vec3 ep = vec3(uv.xy, currentPixelDepth);

    // get the normal of current fragment
    vec3 norm = currentPixelSample.xyz;

    float bl = 0.0;
    for (int i = 0; i < 10; ++i) {
		// get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
		vec3 ray = rad * reflect(sphere[i], fres);

		// get the depth of the occluder fragment
		vec4 occluderFragment = texture2D(normalMap, ep.xy + sign(dot(ray, norm)) * ray.xy);

		// if depthDifference is negative = occluder is behind current fragment
		float depthDifference = currentPixelDepth - occluderFragment.a;

		// the falloff equation, starts at falloff and is kind of 1/x^2 falling
		bl += step(falloff, depthDifference) *
			  (1.0 - dot(occluderFragment.xyz, norm)) *
		      (1.0 - smoothstep(falloff, strength, depthDifference));
    }

    // output the result
    gl_FragColor.r = 1.0 - bl * invSamples;
}
