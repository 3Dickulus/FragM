#donotrun
#include "3D.frag"

#group Post
// for rendering depth to alpha channel in EXR images
// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
uniform bool DepthToAlpha; checkbox[false];
bool depthFlag = true; // do depth on the first hit not on reflections

#group Raytracer

// Distance to object at which raymarching stops.
uniform float Detail;slider[-7,-2.3,0];
// The step size when sampling AO (set to 0 for old AO)
uniform float DetailAO;slider[-7,-0.5,0];

const float ClarityPower = 1.0;

// Lower this if the system is missing details
uniform float FudgeFactor;slider[0,1,1];

float minDist = pow(10.0,Detail);
float aoEps = pow(10.0,DetailAO);
float MaxDistance = 100.0;

// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,56,2000]

// Use this to boost Ambient Occlusion and Glow
//uniform float  MaxRayStepsDiv;  slider[0,1.8,10]

// Used to speed up and improve calculation
uniform float BoundingSphere;slider[0,12,100];

// Can be used to remove banding
uniform float Dither;slider[0,0.5,1];

// Used to prevent normals from being evaluated inside objects.
uniform float NormalBackStep; slider[0,1,10] Locked

#group Light

// AO based on the number of raymarching steps
uniform vec4 AO; color[0,0.7,1,0.0,0.0,0.0];

uniform vec4 CamLight; color[0,1,2,1.0,1.0,1.0];
// Controls the minimum ambient light, regardless of directionality
uniform float CamLightMin; slider[0.0,0.0,1.0]
// Glow based on distance from fractal
uniform vec4 Glow; color[0,0.0,1,1.0,1.0,1.0];
//uniform vec4 InnerGlow; color[0,0.0,1,1.0,1.0,1.0];
uniform int GlowMax; slider[0,20,1000]
// Adds fog based on distance
uniform float Fog; slider[0,0.0,2]

uniform float Shadow; slider[0,0,1]



uniform sampler2D Background; file[Ditch-River_2k.hdr]
uniform sampler2D Specular; file[Ditch-River_2k.hdr]
uniform sampler2D Diffuse; file[Ditch-River_Env.hdr]

uniform float EnvSpecular; slider[0,1,1]
uniform float EnvDiffuse;slider[0,1,1]
// Limits the maximum specular strength to avoid artifacts
uniform float SpecularMax; slider[0,10,100]
uniform vec2 Sun; slider[(-3.1415,-1.57),(0,0),(3.1415,1.57)]
uniform float SunSize; slider[0,0.01,0.4]
uniform bool DebugSun; checkbox[true]
uniform vec2 RotateMap; slider[(0,0),(1,1),(1,1)]
vec4 orbitTrap = vec4(10000.0);
float fractionalCount = 0.0;

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

#ifdef providesNormal
vec3 normal(vec3 pos, float normalDistance);

#else
vec3 normal(vec3 pos, float normalDistance) {
	normalDistance = max(normalDistance*0.5, 1.0e-7);
	vec3 e = vec3(0.0,normalDistance,0.0);
	vec3 n = vec3(DE(pos+e.yxx)-DE(pos-e.yxx),
		DE(pos+e.xyx)-DE(pos-e.xyx),
		DE(pos+e.xxy)-DE(pos-e.xxy));
	n =  normalize(n);
	return n;
}
#endif

#group Floor


#define PI  3.14159265358979323846264

vec2 spherical(vec3 dir) {
	return vec2( acos(dir.z)/PI, atan(dir.y,dir.x)/(2.0*PI) );
}

vec3 fromPhiTheta(vec2 p) {
	return vec3(
		cos(p.x)*sin(p.y),
		sin(p.x)*sin(p.y),
		cos(p.y));
}

uniform bool EnableFloor; checkbox[false]
uniform vec3 FloorNormal; slider[(-1,-1,-1),(0,0,1),(1,1,1)]
uniform float FloorHeight; slider[-5,-1,5]
uniform vec3 FloorColor; color[1,1,1]
uniform bool ShowFloor; checkbox[false]
bool floorHit = false;
float floorDist = 0.0;
vec3 floorNormal = normalize(FloorNormal);
float fSteps = 0.0;
float DEF(vec3 p) {
	float d = DE(p);
	if (EnableFloor) {
		floorDist = abs(dot(floorNormal,p)-FloorHeight);
		if (d<floorDist) {
			fSteps++;
			return d;
		}  else return floorDist;
	} else {
		fSteps++;
		return d;
	}
}

float DEF2(vec3 p) {
	if (EnableFloor) {
		floorDist = abs(dot(floorNormal,p)-FloorHeight);
		return min(floorDist, DE(p));
	} else {
		return DE(p);
	}
}


float shadow(vec3 pos, float eps) {
	vec3 sunDir = fromPhiTheta(Sun);

	// create orthogonal vector (fails for z,y = 0)
	vec3 o1 = normalize( vec3(0., -sunDir.z, sunDir.y));
	vec3 o2 = normalize(cross(sunDir, o1));

	// Convert to spherical coords aliigned to sunDir;
	vec2 r = rand2(viewCoord*(float(subframe)+1.0));
	r.x=r.x*2.*PI;
	r.y= 1.0-r.y*SunSize;
	float oneminus = sqrt(1.0-r.y*r.y);
	vec3 sdir = cos(r.x)*oneminus*o1+
	sin(r.x)*oneminus*o2+
	r.y*sunDir;

	float totalDist = 3.*eps;
	for (int steps=0; steps<MaxRaySteps && totalDist<MaxDistance; steps++) {
		vec3 p = pos + totalDist * sdir;
		float dist = DE(p);
		if (dist < eps)  return 0.0;
		totalDist += dist;
	}
	return 1.0;
}

// float rand(vec2 co){
// 	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
// 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
// }

vec3 equirectangularMap(sampler2D sampler, vec3 dir) {
	// Convert (normalized) dir to spherical coordinates.
	dir = normalize(dir);
	vec2 longlat = vec2(atan(dir.y,dir.x),acos(dir.z));
	// Normalize, and lookup in equirectangular map.
 	return texture2D(sampler,RotateMap+longlat/vec2(2.0*PI,PI)).xyz;
}


vec3 lighting(vec3 n, vec3 color, vec3 pos, vec3 dir, float eps, out float shadowStrength) {
	shadowStrength = 0.0;
	float ambient = max(CamLightMin,dot(-n, dir))*CamLight.w;
	vec3 reflected = -2.0*dot(dir,n)*n+dir;
	vec3 diffuse =  EnvDiffuse*equirectangularMap(Diffuse,n);
	vec3 specular = EnvSpecular*equirectangularMap(Specular,reflected);
	specular = min(vec3(SpecularMax),specular);

	if (Shadow>0.0) {
		// check path from pos to spotDir
		shadowStrength = 1.0-shadow(pos+n*eps,eps);
		ambient = mix(ambient,0.0,Shadow*shadowStrength);
		diffuse = mix(diffuse,vec3(0.0),Shadow*shadowStrength);
		specular = mix(specular,vec3(0.0),Shadow*shadowStrength);
	}

	return (diffuse+CamLight.xyz*ambient)*color+specular;
}

vec3 colorBase = vec3(0.0,0.0,0.0);

vec3 cycle(vec3 c, float s) {
	return vec3(0.5)+0.5*vec3(cos(s*Cycles+c.x),cos(s*Cycles+c.y),cos(s*Cycles+c.z));
}

// Ambient occlusion approximation.
// Sample proximity at a few points in the direction of the normal.
float ambientOcclusion(vec3 p, vec3 n) {
	float ao = 0.0;
	float de = DEF(p);
	float wSum = 0.0;
	float w = 1.0;
	float d = 1.0-(Dither*rand(p.xy));
	for (float i =1.0; i <6.0; i++) {
		// D is the distance estimate difference.
		// If we move 'n' units in the normal direction,
		// we would expect the DE difference to be 'n' larger -
		// unless there is some obstructing geometry in place
		float D = (DEF(p+ d*n*i*i*aoEps) -de)/(d*i*i*aoEps);
		w *= 0.6;
		ao += w*clamp(1.0-D,0.0,1.0);
		wSum += w;
	}
	return clamp(AO.w*ao/wSum, 0.0, 1.0);
}

vec3 getColor() {
	orbitTrap.w = sqrt(orbitTrap.w);

	vec3 orbitColor;
	if (CycleColors) {
		orbitColor = cycle(X.xyz,orbitTrap.x)*X.w*orbitTrap.x +
		cycle(Y.xyz,orbitTrap.y)*Y.w*orbitTrap.y +
		cycle(Z.xyz,orbitTrap.z)*Z.w*orbitTrap.z +
		cycle(R.xyz,orbitTrap.w)*R.w*orbitTrap.w;
	} else {
		orbitColor = X.xyz*X.w*orbitTrap.x +
		Y.xyz*Y.w*orbitTrap.y +
		Z.xyz*Z.w*orbitTrap.z +
		R.xyz*R.w*orbitTrap.w;
	}

	vec3 color = mix(BaseColor, 3.0*orbitColor,  OrbitStrength);
	return color;
}

#ifdef  providesColor
vec3 baseColor(vec3 point, vec3 normal);
#endif


vec3 trace(vec3 from, vec3 dir, inout vec3 hit, inout vec3 hitNormal) {
	hit = vec3(0.0);
	orbitTrap = vec4(10000.0);
	vec3 direction = normalize(dir);
	floorHit = false;
	floorDist = 0.0;

	float totalDist = 0.0;
	float dist = 0.0;

	int steps;
	colorBase = vec3(0.0,0.0,0.0);

	// Check for bounding sphere
	float d = 0.0;
	fSteps = 0.0;

	// We will adjust the minimum distance based on the current zoom
	float eps = minDist; // *zoom;//*( length(zoom)/0.01 );
	float epsModified = 0.0;
	for (steps=0; steps<MaxRaySteps; steps++) {
		orbitTrap = vec4(10000.0);
		vec3 p = from + totalDist * direction;
		dist = DEF(p);
		dist *= FudgeFactor;
		if (steps == 0) dist*=(Dither*rand(direction.xy))+(1.0-Dither);
		totalDist += dist;
		epsModified = pow(totalDist,ClarityPower)*eps;
		if (dist < epsModified) break;
		if (totalDist > MaxDistance) {
			fSteps -= (totalDist-MaxDistance)/dist;
			break;
		}
	}
	if (EnableFloor && dist ==floorDist*FudgeFactor) floorHit = true;
	vec3 hitColor;
	float stepFactor = clamp((fSteps)/float(GlowMax),0.0,1.0);
	vec3 backColor = equirectangularMap(Background, dir);

	if (  steps==MaxRaySteps) orbitTrap = vec4(0.0);

	float shadowStrength = 0.0;
	if ( dist < epsModified) {
		// We hit something, or reached MaxRaySteps
		hit = from + totalDist * direction;
		float ao = AO.w*stepFactor ;

		if (floorHit) {
			hitNormal = floorNormal;
			if (dot(hitNormal,direction)>0.0) hitNormal *=-1.0;
		} else {
			hitNormal= normal(hit-NormalBackStep*epsModified*direction, epsModified); // /*normalE*epsModified/eps*/
		}

#ifdef  providesColor
		hitColor = mix(BaseColor, baseColor(hit,hitNormal),  OrbitStrength);
#else
		hitColor = getColor();
#endif
		if (DetailAO<0.0) ao = ambientOcclusion(hit, hitNormal);

		if (floorHit) {
			hitColor = equirectangularMap(Background, dir);
			if (ShowFloor) {
				vec3 proj = hit-dot(hit, FloorNormal)*hit;
				if (mod(proj.x,1.0)<0.1 || mod(proj.y,1.0)<0.1)
				hitColor = vec3(0.0);
				if (mod(proj.x,1.0)<0.03 || mod(proj.y,1.0)<0.03)
				hitColor = vec3(1.0);
			}
		}
		hitColor = mix(hitColor, AO.xyz ,ao);

		// OpenGL  GL_EXP2 like fog
		//		float f = totalDist;
		//	hitColor = mix(hitColor, backColor, 1.0-exp(-pow(Fog,4.0)*f*f));
		if (floorHit ) {
			shadowStrength = mix(1.0, shadow(hit+hitNormal*epsModified,epsModified),Shadow);
			hitColor*=shadowStrength;
			hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		}else {
			hitColor = lighting(hitNormal, hitColor,  hit,  direction,epsModified,shadowStrength);
		}
	}
	else {
		vec3 col = equirectangularMap(Background, dir);
		if (DebugSun && dot(fromPhiTheta(Sun),normalize(dir))>1.0-SunSize) col= vec3(100.,0.,0.);
		hitColor = col;
		//hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
	}

	if(depthFlag) {
		// do depth on the first hit not on reflections
		depthFlag=false;
		// for rendering depth to alpha channel in EXR images
		// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
		if(DepthToAlpha==true) gl_FragDepth = 1.0/totalDist;
		else
		// sets depth for spline path occlusion
		// see http://www.fractalforums.com/index.php?topic=16405.0
		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
		clamp(totalDist, 0.00001, 1000.0));
	}

	return hitColor;
}

vec3 color(vec3 from, vec3 dir) {
	vec3 hit = vec3(0.0);
	vec3 hitNormal = vec3(0.0);
        depthFlag=true; // do depth on the first hit not on reflections
	return  trace(from,dir,hit,hitNormal);
}
