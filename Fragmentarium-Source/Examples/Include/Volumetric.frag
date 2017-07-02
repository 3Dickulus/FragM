#donotrun
#include "3D.frag"
#include "Kaliset3D.frag"

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

float colorindex=0.;
vec3 coloroverride=vec3(0.);

float minDist = pow(10.0,Detail);
float aoEps = pow(10.0,DetailAO);
//float MaxDistance = 100.0;
vec3 pos;

// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,56,2000]
uniform int MaxDistance;  slider[0,50,2000]
uniform int MinDistance;  slider[0,50,50]


// Use this to boost Ambient Occlusion and Glow
//uniform float  MaxRayStepsDiv;  slider[0,1.8,10]

uniform float VolumetricRayStep;slider[0.01,0.1,1.];
uniform float VolumetricDither;slider[0,.01,.2];


// Can be used to remove banding
uniform float Dither;slider[0,0.5,1];

// Used to prevent normals from being evaluated inside objects.

uniform float NormalBackStep; slider[0,1,10] Locked

uniform float DECheckStep; slider[0,1,100]
uniform float DECheckMaxNormalDiff; slider[0,1,1]



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

uniform float Reflection; slider[0,0,1] Locked
uniform bool DebugSun; checkbox[false] Locked
vec4 orbitTrap = vec4(10000.0);
float fractionalCount = 0.0;

#group PosLight
uniform vec3 LightPos; slider[(-10,-10,-10),(0,0,0),(10,10,10)]
uniform float LightSize; slider[0.00,1,1]
uniform float LightGlowRad; slider[0.0,0.0,5.0]
uniform float LightGlowExp; slider[0.0,1.0,5.0]
uniform float LightGlowIntensity; slider[0.0,0.0,1.0]
uniform float LightGlowClear; slider[0.0,0.0,10.0]
uniform float VolumetricShadow; slider[0,0,5]
uniform float MinLight; slider[0.0,0.0,1.0]



#group Coloring

uniform vec3 BaseColor; color[1.0,1.0,1.0];
uniform vec3 FogColor; color[1.0,1.0,1.0];
uniform float ColoringMix; slider[0,.5,1]
uniform float ColorDensity; slider[0,.5,1]
uniform float ColorOffset; slider[0,0,1]
uniform vec3 Color1; color[1.0,0.0,0.0]
uniform vec3 Color2; color[0.0,1.0,0.0]
uniform vec3 Color3; color[0.0,0.0,1.0]
uniform vec3 Color4; color[1.0,0.5,0.5]
uniform vec3 Color5; color[0.5,0.1,0.5]
uniform vec3 Color6; color[0.5,0.5,1.0]

// Background color
uniform vec3 BackgroundColor; color[0.6,0.6,0.45]
// Vignette background
uniform float GradientBackground; slider[0.0,0.3,5.0]

float DE(vec3 pos) ; // Must be implemented in other file
float DElight(vec3 pos) {
	return length(pos+LightPos)-LightSize;
}

float DEL(vec3 pos, inout float lightde) {
	lightde=DElight(pos);
	return min(DE(pos)*FudgeFactor,lightde);
}

float DEL2(vec3 pos, inout float lightde) {
	lightde=DElight(pos);
	return min(DE(pos),lightde);
}



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



// Uses the soft-shadow approach by Quilez:
// http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
float shadow(vec3 pos, vec3 sdir, float eps) {
	float totalDist =2.0*eps;
	float s = 1.0; // where 1.0 means no shadow!
	float lightde=0.0;
	float dist;
	for (int steps=0; steps<MaxRaySteps; steps++) {
		vec3 p = pos + totalDist * sdir;
		dist = DEL2(p, lightde);
		if (dist < eps) return dist==lightde?0.:1.;
		totalDist += dist;
	}
	return 0.;
}

float rand(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 lightnormal(vec3 pos, float normalDistance) {
	normalDistance = max(normalDistance*0.5, 1.0e-7);
	vec3 e = vec3(0.0,normalDistance,0.0);
	vec3 n = vec3(DElight(pos+e.yxx)-DElight(pos-e.yxx),
		DElight(pos+e.xyx)-DElight(pos-e.xyx),
		DElight(pos+e.xxy)-DElight(pos-e.xxy));
	n =  normalize(n);
	return n;
}



vec3 lighting(vec3 n, vec3 color, vec3 pos, vec3 dir, float eps, out float shadowStrength) {
	shadowStrength = 0.0;
	//	vec3 spotDir = vec3(sin(SpotLightDir.x*3.1415)*cos(SpotLightDir.y*3.1415/2.0), sin(SpotLightDir.y*3.1415/2.0)*sin(SpotLightDir.x*3.1415), cos(SpotLightDir.x*3.1415));
	//	spotDir = normalize(spotDir);
	vec3 spotDir = -lightnormal(pos,eps);
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
		ambient = mix(ambient,0.0,HardShadow*shadowStrength);
		diffuse = mix(diffuse,0.0,HardShadow*shadowStrength);
		// specular = mix(specular,0.0,HardShadow*f);
		if (shadowStrength>0.0) specular = 0.0; // always turn off specular, if blocked
	}

	return (SpotLight.xyz*diffuse+CamLight.xyz*ambient+ specular*SpotLight.xyz)*color;
}

vec3 colorBase = vec3(0.0,0.0,0.0);


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


vec3 getColor(float index) {
	float cx=index*ColorDensity+ColorOffset*PI*6;
	vec3 col;
	float ps=PI/3;
	float f1=max(0,sin(cx)-.5)/.5;
	float f2=max(0,sin(cx+ps)-.5)/.5;
	float f3=max(0,sin(cx+ps*2)-.5)/.5;
	float f4=max(0,sin(cx+ps*3)-.5)/.5;
	float f5=max(0,sin(cx+ps*4)-.5)/.5;
	float f6=max(0,sin(cx+ps*5)-.5)/.5;
	col=mix(Color1,Color2,f1);
	col=mix(col,Color3,f2);
	col=mix(col,Color4,f3);
	col=mix(col,Color5,f4);
	col=mix(col,mix(Color6,Color1,f6),f5);
	col=mix(BaseColor,col,ColoringMix);
	if (index<0.) col=coloroverride;
	return col;
}


#ifdef  providesColor
vec3 baseColor(vec3 point, vec3 normal);
#endif

float epsModified = 0.0;

bool lighthit=false;


vec3 trace(vec3 from, vec3 dir, inout vec3 hit, inout vec3 hitNormal, inout float glow) {
	glow=1000.0;
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
	float lightde;
	lighthit=false;

	for (steps=0; steps<MaxRaySteps; steps++) {
		orbitTrap = vec4(10000.0);
		vec3 p = from + totalDist * direction;
		dist = DEL(p, lightde);
		glow=min(lightde,glow);
		if (steps == 0) dist*=(Dither*rand(direction.xy))+(1.0-Dither);
		totalDist += dist;
		epsModified = pow(totalDist,ClarityPower)*0.+eps;
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
		if (dist==lightde) {lighthit=true; return SpotLight.xyz*SpotLight.w*2;}

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
		hitColor = getColor(colorindex);
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
			vec3 spotDir = vec3(sin(SpotLightDir.x*3.1415)*cos(SpotLightDir.y*3.1415/2.0), sin(SpotLightDir.y*3.1415/2.0)*sin(SpotLightDir.x*3.1415), cos(SpotLightDir.x*3.1415));
			spotDir = normalize(spotDir);
			if (dot(spotDir,normalize(dir))>0.9) hitColor= vec3(100.,0.,0.);
		}
	}

	if(depthFlag) {
                 // do depth on the first hit not on reflections
		depthFlag=false; // only do it once
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
	vec3 color = vec3(0.0);
	vec3 hit = vec3(0.0);
	vec3 hitNormal = vec3(0.0);
	float l=0;
	float hshadow=0.0;
	float glow=0;
	float ladjust=1;
        depthFlag=true; // do depth on the first hit not on reflections
	vec3 tr=trace(from,dir,hit,hitNormal,glow);
	if (hitNormal== vec3(0.0)) {hit=from+dir*MaxDistance;}
	color=tr;
	from+=((rand(dir.xy+vec2(subframe,subframe*1.12358))-.5)*VolumetricDither);
	float dist=length(from-hit);
	int c=0;
	float lf=0;
	float sh=0;
	if (!lighthit && LightGlowIntensity>0) {
		for (float r=MinDistance; r<dist && r<MaxDistance; r+=VolumetricRayStep) {
			vec3 p=from+r*dir;
			vec3 lightdir=-lightnormal(p,.01);
			float ep=exp(-(LightGlowClear*.01)*r*r);
			float k=max(MinLight,Kaliset(p)*KFinalScale);
			//sh=shadow(p,lightdir,minDist*10);
			lf+=k*ep*clamp((1-sh),VolumetricShadow,1.);
			c++;
		}
	}
	lf=max(0,lf);
	lf*=VolumetricRayStep;
	lf*=LightGlowIntensity*5;

	//	vec3 FColor=max(vec3(0),FogColor-vec3(sh));
	vec3 FColor=FogColor*lf;
	//	color=mix(color,FColor,1.0-e);
	float glow1=exp(-pow(1/LightGlowRad,4)*pow(glow,LightGlowExp));
	float glow2=exp(-10*pow(glow,1));
	//	color=mix(color,SpotLight.xyz*glow1*SpotLight.w,lf)+SpotLight.xyz*glow2*SpotLight.w*2;
	color+=SpotLight.xyz*glow1*SpotLight.w*lf+SpotLight.xyz*glow2*SpotLight.w*2;
	//	color=mix(color,(SpotLight.xyz*glow)*lf*SpotLight.w*2,SpotLight.w);
	//	color=mix(color,BackgroundColor,1.0-e);
	return color;
}
