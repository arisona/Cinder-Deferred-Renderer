#version 120

//input parameters
uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D attributeMap;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float dist;

varying vec4 pos;

// XXX couldn't we pass uv from vert shader, as with others?

void main(void) {
	vec2 uv = pos.xy / pos.w * 0.5 + 0.5;
	vec4 p = texture2D(positionMap, uv);			// get the position from deferred shading

	vec3 vlp = lightPosition - p.xyz;				// vector between light and point
	float d = length(vlp);							// get the distance between the light and point
	if (d > dist) discard;						// if outside of area of effect, discard pixel

	vlp /= d;										// normalize vector between light and point (divide by distance)
	vec4 norm = texture2D(normalMap, uv);			// get the normal from deferred shading
	vec4 col = texture2D(colorMap, uv);				// get the color from deferred shading
	vec4 attr = texture2D(attributeMap, uv);		// get lighting attributes from deferred shading
	float diff_coeff = attr.r;
	float phong_coeff = attr.g;
	float two_sided = attr.b;
	float cost = dot(norm.xyz, vlp);
	// XXX we need something like this... if (cost < 0) discard;
	cost = (cost < 0.0) ? -two_sided * cost : cost;						// calculate two sided lighting.
	float diff = diff_coeff * cost;										// calculate diffuse shading
	vec3 h = normalize(vlp + normalize(cameraPosition - p.xyz));		// calculate half vector
	float phong = phong_coeff * pow(max(dot(h, norm.xyz), 0.0), 100.0);	// calculate phong shading
	vec3 c = lightColor * (col.rgb * diff + phong) / (d * d + 0.8);		// calculate light contribution with attenuation
																		// (all lights have constant quadratic attenuation of 1.0,
																		// with a constant attenuation of 0.8 to avoid dividing by
																		// small numbers
    gl_FragColor = vec4(c, 1.0);										// output color
}
