#donotrun

#include "3D.frag"

#group Post
// Available when using exr image filename extention
uniform bool DepthToAlpha; checkbox[false];
// for rendering depth to alpha channel in EXR images, set in DE-Raytracer.frag
// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
uniform bool ShowDepth; checkbox[false];
uniform float DepthMagnitude;slider[0,1,10];

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
uniform float MaxDistance;  slider[0,1000,5000];

// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,56,5000]

// Use this to boost Ambient Occlusion and Glow
//uniform float  MaxRayStepsDiv;  slider[0,1.8,10]

// Can be used to remove banding
uniform float Dither;slider[0,0.5,1];

// Used to prevent normals from being evaluated inside objects.
uniform float NormalBackStep; slider[0,1,10] Locked

#group Light

// AO based on the number of raymarching steps
uniform vec4 AO; color[0,0.7,1,0.0,0.0,0.0];

// The specular intensity of the directional light
uniform float Specular; slider[0,0.4,1.0];
// The specular exponent
uniform float SpecularExp; slider[0,16.0,100.0];
// Limits the maximum specular strength to avoid artifacts
uniform float SpecularMax; slider[0,10,100]
// Color and strength of the directional light
uniform vec4 SpotLight; color[0.0,0.4,1.0,1.0,1.0,1.0];
// Direction to the spot light (spherical coordinates)
uniform vec2 SpotLightDir;  slider[(-1,-1),(0.1,0.1),(1,1)]
// Light coming from the camera position (diffuse lightning)
uniform vec4 CamLight; color[0,1,2,1.0,1.0,1.0];
// Controls the minimum ambient light, regardless of directionality
uniform float CamLightMin; slider[0.0,0.0,1.0]
// Glow based on distance from fractal
uniform vec4 Glow; color[0,0.0,1,1.0,1.0,1.0];
//uniform vec4 InnerGlow; color[0,0.0,1,1.0,1.0,1.0];
uniform int GlowMax; slider[0,20,1000]
// Adds fog based on distance
uniform float Fog; slider[0,0.0,2]
// Hard shadows shape is controlled by SpotLightDir
uniform float HardShadow; slider[0,0,1] Locked

uniform float ShadowSoft; slider[0.0,2.0,20]

uniform bool QualityShadows; checkbox[false]

uniform float Reflection; slider[0,0,1] Locked
uniform bool DebugSun; checkbox[false] Locked
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

#ifdef providesBackground
vec3  backgroundColor(vec3 dir);
#endif

#group Floor

uniform bool EnableFloor; checkbox[false] Locked
uniform vec3 FloorNormal; slider[(-1,-1,-1),(0,0,1),(1,1,1)]
uniform float FloorHeight; slider[-5,0,5]
uniform vec3 FloorColor; color[1,1,1]
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

float rand(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Uses the soft-shadow approach by Quilez:
// http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
float shadow(vec3 pos, vec3 sdir, float eps) {
	float totalDist;

	vec3 sdir1 = sdir;

	float s1,s2,s3;
	s1=s2=s3=1.0;// where 1.0 means no shadow!

	float ShadowSoft = ShadowSoft;
	if(QualityShadows) {
		ShadowSoft *= 2.;

		vec3 perp;
		if(sdir.x/length(sdir) > .9) perp = cross(sdir, vec3(0.0,1.0,0.0));
		else perp = cross(sdir, vec3(1.0,0.0,0.0));
		vec3 perp2 = cross(sdir,perp);

		float angle1 = PI*rand(vec2(pos.x,pos.y+pos.z));
		float angle2 = angle1 + (2.0/3.0*PI);
		float angle3 = angle1 + (4.0/3.0*PI);

		float fac = .5/ShadowSoft*rand(vec2(pos.y,pos.x+pos.z));
		sdir1 = sdir + fac*perp*cos(angle1) + fac*perp2*sin(angle1);
		vec3 sdir2 = sdir + fac*perp*cos(angle2) + fac*perp2*sin(angle2);
		vec3 sdir3 = sdir + fac*perp*cos(angle3) + fac*perp2*sin(angle3);

		 totalDist =2.0*eps;
		for (int steps=0; steps<MaxRaySteps && totalDist<MaxDistance; steps++) {
			vec3 p = pos + totalDist * sdir2;
			float dist = DEF2(p);
			if (dist < eps)  return 1.0;
			s2 = min(s2, 30.*ShadowSoft*dist/totalDist);
			totalDist += dist;
		}
				 totalDist =2.0*eps;
		for (int steps=0; steps<MaxRaySteps && totalDist<MaxDistance; steps++) {
			vec3 p = pos + totalDist * sdir2;
			float dist = DEF2(p);
			if (dist < eps)  return 1.0;
			s3 = min(s3, 30.*ShadowSoft*dist/totalDist);
			totalDist += dist;
		}
	}
	 totalDist =2.0*eps;
	for (int steps=0; steps<MaxRaySteps && totalDist<MaxDistance; steps++) {
		vec3 p = pos + totalDist * sdir1;
		float dist = DEF2(p);
		if (dist < eps)  return 1.0;
		s1 = min(s1, 30.*ShadowSoft*dist/totalDist);
		totalDist += dist;
	}
	float s = QualityShadows ? (s1+s2+s3)/3.0 : s1;
	return 1.0-s;
}

float piOverTwo = PI/2.0;

vec3 lighting(vec3 n, vec3 color, vec3 pos, vec3 dir, float eps, out float shadowStrength) {
	shadowStrength = 0.0;
	vec3 spotDir = vec3(sin(SpotLightDir.x*PI)*cos(SpotLightDir.y*piOverTwo), sin(SpotLightDir.y*piOverTwo)*sin(SpotLightDir.x*PI), cos(SpotLightDir.x*PI));
	spotDir = normalize(spotDir);

	float nDotL = max(0.0,dot(n,spotDir));
	vec3 halfVector = normalize(-dir+spotDir);
	float diffuse = nDotL*SpotLight.w;
	float ambient = max(CamLightMin,dot(-n, dir))*CamLight.w;
	float hDotN = max(0.,dot(n,halfVector));

	// An attempt at Physcical Based Specular Shading:
	// http://renderwonk.com/publications/s2010-shading-course/
	// (Blinn-Phong with Schickl term and physical normalization)
	float specular =((SpecularExp+2.)/8.)*pow(hDotN,SpecularExp)*
	(SpecularExp + (1.-SpecularExp)*pow(1.-hDotN,5.))*
	nDotL*Specular;
	specular = min(SpecularMax,specular);

	if (HardShadow>0.0) {
		// check path from pos to spotDir
		shadowStrength = shadow(pos+n*eps, spotDir, eps);
		//ambient = mix(ambient,0.0,HardShadow*shadowStrength);
		diffuse = mix(diffuse,0.0,HardShadow*shadowStrength);
		// specular = mix(specular,0.0,HardShadow*f);
		if (shadowStrength>0.0) specular = 0.0; // always turn off specular, if blocked
	}

	return (SpotLight.xyz*diffuse+CamLight.xyz*ambient+ specular*SpotLight.xyz)*color;
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

	float dist = 0.0;
	float totalDist = 0.0;

	int steps;
	colorBase = vec3(0.0,0.0,0.0);


	// We will adjust the minimum distance based on the current zoom
	float eps = minDist;
	float epsModified = 0.0;

	for (steps=0; steps<MaxRaySteps; steps++) {
		orbitTrap = vec4(10000.0);
		vec3 p = from + totalDist * direction;
		dist = DEF(p);
		dist *= FudgeFactor;

		if (steps == 0) dist*=(Dither*rand(direction.xy))+(1.0-Dither);
		totalDist += dist;
		epsModified = pow(totalDist,ClarityPower)*eps;
		if (dist < epsModified) {
			// move back
			totalDist -= (epsModified-dist);
			break;
		}
		if (totalDist > MaxDistance) {
			fSteps -= (totalDist-MaxDistance)/dist;
			break;
		}
	}
	if (EnableFloor && dist ==floorDist*FudgeFactor) floorHit = true;
	vec3 hitColor;
	float stepFactor = clamp((fSteps)/float(GlowMax),0.0,1.0);
	vec3 backColor = BackgroundColor;
	if (GradientBackground>0.0) {
		float t = length(coord);
		backColor = mix(backColor, vec3(0.0,0.0,0.0), t*GradientBackground);
	}

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
		hitColor = mix(BaseColor,  baseColor(hit,hitNormal),  OrbitStrength);
		#else
		hitColor = getColor();
		#endif
		#ifndef linearGamma
		hitColor = pow(clamp(hitColor,0.0,1.0),vec3(Gamma));
		#endif
		if (DetailAO<0.0) ao = ambientOcclusion(hit, hitNormal);
		if (floorHit) {
			hitColor = FloorColor;
		}

		hitColor = mix(hitColor, AO.xyz ,ao);
		hitColor = lighting(hitNormal, hitColor,  hit,  direction,epsModified,shadowStrength);
		// OpenGL  GL_EXP2 like fog
		float f = totalDist;
		hitColor = mix(hitColor, backColor, 1.0-exp(-pow(Fog,4.0)*f*f));
		if (floorHit ) {
			hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		}
	}
	else {
		#ifdef providesBackground
		hitColor = backgroundColor(dir);
		#else
		hitColor = backColor;
		#endif
		hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		hitNormal = vec3(0.0);
		if (DebugSun) {
			vec3 spotDir = vec3(sin(SpotLightDir.x*PI)*cos(SpotLightDir.y*piOverTwo), sin(SpotLightDir.y*piOverTwo)*sin(SpotLightDir.x*PI), cos(SpotLightDir.x*PI));
			spotDir = normalize(spotDir);
			if (dot(spotDir,normalize(dir))>0.9) hitColor= vec3(100.,0.,0.);
		}
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
// 		gl_FragDepth = (1.0 + (-1e-05 / clamp (totalDist, 1e-05, 1.0)));
		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
		clamp(totalDist, 0.00001, 1000.0));
	}
	if(ShowDepth) hitColor = vec3(1.0)-vec3(1.0/totalDist)*DepthMagnitude;
	return hitColor;
}

vec3 color(vec3 from, vec3 dir) {
	vec3 hit = vec3(0.0);
	vec3 hitNormal = vec3(0.0);
    depthFlag=true; // do depth on the first hit not on reflections
	vec3 first =  trace(from,dir,hit,hitNormal);

	if (Reflection==0. || hitNormal == vec3(0.0)) {
		return first;
	} else {
		vec3 d = reflect(dir, hitNormal);
		return mix(first,trace(hit+d*minDist,d,hit, hitNormal),Reflection);
	}
}
