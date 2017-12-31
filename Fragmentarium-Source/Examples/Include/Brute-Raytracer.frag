#donotrun

#include "Brute3D.frag"

#group Camera


#group Raytracer

uniform float NormalScale;slider[0,1,5]
uniform float AOScale;slider[0,1,5]
uniform float Glow;slider[0,0.1,1]
uniform float AOStrength;slider[0,0.6,1]

// Maximum number of  raymarching steps.
uniform int Samples;  slider[0,40,2000]
uniform bool Stratify; checkbox[true]
uniform bool DebugInside; checkbox[false]
uniform bool CentralDifferences; checkbox[true]
uniform bool SampleNeighbors; checkbox[true]

uniform float Near; slider[0,0,12]
uniform float Far; slider[0,5,12]

// Available when using exr image filename extention
uniform bool DepthToAlpha; checkbox[false];

uniform bool  DebugNormals; checkbox[false]

#group Light

// The specular intensity of the directional light
uniform float Specular; slider[0,4.0,10.0];
// The specular exponent
uniform float SpecularExp; slider[0,16.0,100.0];
// Color and strength of the directional light
uniform vec4 SpotLight; color[0.0,0.4,1.0,1.0,1.0,1.0];
// Direction to the spot light (spherical coordinates)
uniform vec2 SpotLightDir;  slider[(-1,-1),(0.1,0.1),(1,1)]
// Light coming from the camera position (diffuse lightning)
uniform vec4 CamLight; color[0,0.3,2,1.0,1.0,1.0];
// Controls the minimum ambient light, regardless of directionality
uniform float CamLightMin; slider[0.0,0.0,1.0]

uniform float Fog; slider[0,0.0,2]


vec4 orbitTrap = vec4(10000.0);

#group Coloring

// This is the pure color of object (in white light)
uniform vec3 BaseColor; color[1.0,1.0,1.0];
// Determines the mix between pure light coloring and pure orbit trap coloring
uniform float OrbitStrength; slider[0,0,1]

// Closest distance to YZ-plane during orbit
uniform vec4 X; color[-1,0.7,1,0.5,0.6,0.6];

// Closest distance to XZ-plane during orbit
uniform vec4 Y; color[-1,0.4,1,1.0,0.6,0.0];

// Closest distance to XY-plane during orbit
uniform vec4 Z; color[-1,0.5,1,0.8,0.78,1.0];

// Closest distance to  origin during orbit
uniform vec4 R; color[-1,0.12,1,0.4,0.7,1.0];

// Background color
uniform vec3 BackgroundColor; color[0.6,0.6,0.45]
// Vignette background
uniform float GradientBackground; slider[0.0,0.3,5.0]

float DE(vec3 pos) ; // Must be implemented in other file

uniform bool CycleColors; checkbox[false]
uniform float Cycles; slider[0.1,1.1,32.3]

#group Floor

vec3 colorBase = vec3(0.0,0.0,0.0);

vec3 getColor() {
	orbitTrap.w = sqrt(orbitTrap.w);

	vec3 orbitColor;
	orbitColor = X.xyz*X.w*orbitTrap.x +
	Y.xyz*Y.w*orbitTrap.y +
	Z.xyz*Z.w*orbitTrap.z +
	R.xyz*R.w*orbitTrap.w;

	vec3 color = mix(BaseColor, 3.0*orbitColor,  OrbitStrength);
	return color;
}

// float rand(vec2 co){
// 	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
// 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
// }

#ifdef  providesColor
vec3 color(vec3 point);
#endif

#ifdef providesInside
bool inside(vec3 p);
#else
bool inside(vec3 p) {
	return (DE(p) < 0.0);
}
#endif

uniform vec2 pixelSize;
vec4 color(vec3 from, vec3 dir, float closest) {
	vec3 direction = normalize(dir);
	float dist = 0.0;
	if (closest<=0. || closest>1.) { closest = 1.0; }

	bool startsInside = inside(from + Near * direction);


	// Check neighbors
	if (SampleNeighbors && subframe>1) {
		vec2 position = (viewCoord*1.0+vec2(1.0))/2.0;

		for (int dx = -1; dx<=1; dx++) {
			for (int dy = -1; dy<=1; dy++) {
				float dist = texture2D( backbuffer,  position + pixelSize*vec2( dx, dy ) ).w;
				if (dist < closest) {
					vec3 point = from + (Near+dist*(Far-Near)) * direction;
					if (inside(point) != startsInside) {
						closest = dist;
					}
				}
			}
		}
	}

	if (Stratify) {
		float stepSize =  closest / float(Samples);

		float dither= rand(viewCoord*float(subframe));
		dist += dither*stepSize;
		int steps;
		for (steps=0; steps<Samples; steps++) {
			vec3 point = from + (Near+dist*(Far-Near)) * direction;
			if (inside(point) != startsInside) break;
			dist += stepSize;
		}
		if (steps!=Samples) closest = dist;
	} else {

		for (int i=0; i<Samples; i++) {
			dist = closest*rand(viewCoord*float(subframe*i));
			vec3 point = from + (Near+dist*(Far-Near)) * direction;
			if (inside(point) != startsInside) {
				closest = dist;
			}
		}
	}

	if (closest >= 1.0) {
		vec3 backColor = BackgroundColor;
		if (GradientBackground>0.0) {
			float t = length(coord);
			backColor = mix(backColor, vec3(0.0,0.0,0.0), t*GradientBackground);
		}
gl_FragDepth = 1.;
		return vec4(backColor,1.0);
	}

#ifdef  providesColor
	vec3 hitColor = mix(BaseColor,  color( from + (Near+closest*(Far-Near)) * direction),  OrbitStrength);
#else
	vec3 hitColor = getColor();
#endif
	if (DebugInside) {
		if (startsInside) {
			hitColor = vec3(1.0,0.0,0.0);

		} else {
			hitColor = vec3(0.0,1.0,0.0);
		}
	}

	if (DepthToAlpha)
	  gl_FragDepth = 1.0/(length( from + direction * dist)/length(dir));
	else
	gl_FragDepth = ( (1000.0 / (1000.0 - 0.00001)) +
	  (1000.0 * 0.00001 / (0.00001 - 1000.0)) /
	  clamp(length( from + direction * dist)/length(dir), 0.00001, 1000.0) );

	return vec4(hitColor,closest);
}
