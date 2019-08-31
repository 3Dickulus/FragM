#donotrun
#info DE-RaytracerKn-0.9.12.frag: Original shader by syntopia. Modifications by knighty + Eiffie + ChrisJRodgers:
#info  Testing multisampling of color palette or loaded textures..  Benesi
#info  - Added multiple reflections
//Log:
//mar 7 2015:
// - Now the fog is colored and effects lighting.
// - Colored reflections.
//mar 6 2015:
// - Black dots again. Visible especially when using reflections. Got hard time to fix it. :-/. Any time a transcendental function is involved, one have to take care about infinities, denormals and NaNs.
//mar 5 2015:
// - Implemented stratified sampling for volumetric light and "corrected" use of RNG.
//mar 3 2015:
// -Added Iq's cloud implemented by Eifie. In order to use it #define USE_IQ_CLOUDS before including DE-kn2.frag
// -Added volumetric light. In order to use it #define KN_VOLUMETRIC before including DE-kn2.frag.
//mar 1 2015:
// -Updated settings.
// -Tweaked the AO function to make it "multiscale". Work still in progress: Identify and implement pertinent parameteres. See Baird-Delta preset "AO_Alone"
// -Corrected warnings.
//feb 24 2015:
// -Corrected some bugs. Floating point numbers are tricky.
// -Corrected bug with DoF and Floor.
// -Added point kight distance falloff.
// -Modified height fog formula again.
//feb 23 2015:
// -switched to Eiffie's shoft shadow. Looks much better to me.
// -added altitude fog.
// -ChristRodgers added Point light with glow. Modified a bit.
// -Added bloom effect. Bug corrected.
//Known bugs:
// -Still some black spots on some graphics card. Maybe a driver issue. Don't use unrealistic values for DoF effect plz ;o)
//ToDo:
// -Simplify point light glow computations. Low priority: The light glow is good but suffers from banding artifacts (needs high dither values).
// -Implement user defined matrials. High priority.
// -Reduce banding artifacts ue to tracing accuracy. High priority.
// -SoC? Global illumination? (maybe needs complete refactoring and/or removing InFocusAWidth which may complicate things)

#include "3DKn-1.0.1.frag"

#group Post
// Available when using exr image filename extention
uniform bool DepthToAlpha; checkbox[false];
// for rendering depth to alpha channel in EXR images, set in DE-Raytracer.frag
// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
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
//float MaxDistance = 100.0;

// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,56,5000]
uniform float MaxDistance;slider[0,20,1000];
// Use this to boost Ambient Occlusion and Glow
//uniform float  MaxRayStepsDiv;  slider[0,1.8,10]

// Can be used to remove banding
uniform float Dither;slider[0,0.5,1];

// Used to prevent normals from being evaluated inside objects.
uniform float NormalBackStep; slider[0,1,10]

#group Light

// AO based on the number of raymarching steps
uniform vec4 AO; color[0,0.7,1,0.0,0.0,0.0];
uniform float AoCorrect; slider[0.0,0.0,1.0]
// The specular intensity of the directional light
uniform float Specular; slider[0,0.4,2.0];
// The specular exponent
uniform float SpecularExp; slider[0,16.0,500.0];
// Limits the maximum specular strength to avoid artifacts
//uniform float SpecularMax; slider[0,10,100]



// Light coming from the camera position (diffuse lightning)
uniform vec4 CamLight; color[0,1,2,1.0,1.0,1.0];
// Controls the minimum ambient light, regardless of directionality
uniform float CamLightMin; slider[0.0,0.0,1.0]
// Glow based on distance from fractal
uniform vec4 Glow; color[0,0.0,1,1.0,1.0,1.0];
//uniform vec4 InnerGlow; color[0,0.0,1,1.0,1.0,1.0];
uniform int GlowMax; slider[0,20,1000]
// Adds fog based on distance
//uniform float Fog; slider[0,0.0,2]
//uniform float FogExp; slider[0,2,10]

//uniform float ShadowSoft; slider[0.0,2.0,20]
//uniform float ShadowBlur; slider[0.0,0.0,20]
uniform vec3 Reflection; color[1,1,1]
uniform int ReflectionsNumber; slider[0,0,5]
vec4 orbitTrap = vec4(10000.0);
float fractionalCount = 0.0;

#group PosLight
// Color and strength of the directional light
uniform bool SpotGlow; checkbox[true]
uniform vec4 SpotLight; color[0.0,1.,10.0,1.0,1.0,1.0];
uniform vec3 LightPos; slider[(-10,-10,-10),(0,0,0),(10,10,10)]
uniform float LightSize; slider[0.0,0.1,1]
uniform float LightFallOff; slider[0,0,2]
uniform float LightGlowRad; slider[0.0,0.0,5.0]
uniform float LightGlowExp; slider[0.0,1.0,5.0]
// Hard shadows shape is controlled by SpotLightDir
uniform float HardShadow; slider[0,0,1]
uniform float ShadowSoft; slider[0.0,10.0,20]

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

float DElight(vec3 pos) {
	return length(LightPos-pos)-LightSize;
}

float DEL(vec3 pos, inout float lightde) {
	lightde=DElight(pos)-LightSize;
	return min(DE(pos)*FudgeFactor,lightde);
}

float DEL2(vec3 pos, inout float lightde) {
	lightde=DElight(pos)-LightSize;
	return min(DE(pos),lightde);
}


#ifdef providesNormal
vec3 normal(vec3 pos, float normalDistance);

#else
#if 0
vec3 normal(vec3 pos, float normalDistance) {
	normalDistance = max(normalDistance*0.5, 1.0e-5);
	vec3 e = vec3(0.0,normalDistance,0.0);
	vec3 n = vec3(DE(pos+e.yxx)-DE(pos-e.yxx),
		DE(pos+e.xyx)-DE(pos-e.xyx),
		DE(pos+e.xxy)-DE(pos-e.xxy));
	n =  normalize(n);
	return n;
}
#else
vec3 normal(vec3 pos, float normalDistance) {
	normalDistance = max(normalDistance*0.5, 1.0e-5);
	vec3 e = vec3(0.0,normalDistance,0.);
	vec3 n = vec3(DE(pos+e.yxx)-DE(pos-e.yxx),
		DE(pos+e.xyx)-DE(pos-e.xyx),
		DE(pos+e.xxy)-DE(pos-e.xxy));
	n = normalize(n);
	return n==n ? n : vec3(0.0);
}
#endif
#endif

#ifdef providesBackground
vec3  backgroundColor(vec3 dir);
#endif

#group Floor

uniform bool EnableFloor; checkbox[false]
uniform vec3 FloorNormal; slider[(-1,-1,-1),(0,0,1),(1,1,1)]
uniform float FloorHeight; slider[-5,0,5]
uniform vec3 FloorColor; color[1,1,1]
bool floorHit = false;
float floorDist = 0.0;
vec3 floorNormal = normalize(FloorNormal);
float fSteps = 0.0;

#group Height_Fog
uniform float HF_Fallof; slider[0.0005,0.1,5.]
uniform float HF_Const; slider[0.0,0.0,1.0]
uniform float HF_Intensity; slider[0.0,0.,1.0]
uniform vec3 HF_Dir; slider[(-1,-1,-1),(0,0,1),(1,1,1)]
uniform float HF_Offset; slider[-10.,0.,10.0]
uniform vec4 HF_Color; color[0.0,1.,3.0,1.0,1.0,1.0]
#ifdef KN_VOLUMETRIC
//to modify volumetric light intensity
uniform float HF_Scatter; slider[0.0,0.,10.0]
//Anisotropy of the fog.
//uniform float HF_Anisotropy; slider[0.0,0.,1.0]
uniform vec3 HF_Anisotropy; color[0,0,0]
//for stratified sampling. Helps to accelerate convergence but slows down the framerate. Better use it for final render. You'll need less subframes.
uniform int HF_FogIter; slider[1,1,16]
//Is shadow cast for every sample. Slows down the rendering.
uniform bool HF_CastShadow; checkbox[false]
#endif

float exp1(float x){ return exp(clamp(x,-80.,80.));}//have to use this to avoid black dots (again)
vec3 exp1(vec3 x){ return exp(clamp(x,vec3(-80.),vec3(80.)));}
float fogAmount(vec3 P0, vec3 P1){//Modified from: http://www.iquilezles.org/www/articles/fog/fog.htm
	//ToDo: make dependent on the fog color
	vec3 hfdir=normalize(HF_Dir);
	float t=length(P1-P0);
	float A=HF_Fallof*dot(P1-P0,hfdir);
	A=(1.0-exp1(-A))/A;
	float amount= (HF_Intensity * exp1(-HF_Fallof*(dot(P0,hfdir)-HF_Offset))* A  + HF_Const) * t;
	//amount=clamp(amount,-5.,100.);
	return clamp(exp1(-amount),0.,1.);//Return transmission factor
}

vec3 fogAmount3(vec3 P0, vec3 P1){//Modified from: http://www.iquilezles.org/www/articles/fog/fog.htm
	//ToDo: make dependent on the fog color
	vec3 hfdir=normalize(HF_Dir);
	float t=length(P1-P0);
	float A=HF_Fallof*dot(P1-P0,hfdir);
	A=(1.0-exp1(-A))/A;
	vec3 amount= HF_Color.rgb*(HF_Intensity * exp1(-HF_Fallof*(dot(P0,hfdir)-HF_Offset))* A  + HF_Const) * t;
	//amount=clamp(amount,-5.,100.);
	return clamp(exp1(-amount),vec3(0.),vec3(1.));//Return transmission factor
}

vec3 ptLightGlow(float glow){
	if(SpotGlow){
		float glow1=exp(-pow(LightGlowRad+0.0001,-2.)*pow(glow,LightGlowExp));
		float glow2=exp(-20.*pow(glow,1.));
		return SpotLight.xyz*glow2*SpotLight.w*1.+SpotLight.xyz*glow1;
	}
	return vec3(0.);
}


float DEF(vec3 p) {
	float d = DE(p) ;
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

#define MIN_EPS 2./16777216.


// float rand(vec2 co){
// 	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
// 	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
// }

#ifdef USE_EIFFIE_SHADOW
// Uses the soft-shadow approach by Eiffie:
float linstep(float a, float b, float t){return clamp((t-a)/(b-a),0.,1.);}
float shadow(vec3 ro, vec3 lightPos, float eps){//float FuzzyShadow(vec3 ro, vec3 rd, float lightDist, float coneGrad, float rCoC){
	//return 1.;
	float rCoC=eps;
	vec3 rd=(lightPos-ro);
	float lightDist=length(rd);
	rd/=lightDist;
	float coneGrad=ShadowSoft;
	if(ShadowSoft==0.) coneGrad=LightSize/lightDist; else coneGrad=1./ShadowSoft;
	float t=DEF2(ro)+rCoC;//avoid self shadowing
	float d=1.0,s=1.0;
	float jitter=Dither*(rand(ro.xy*(float(subframe)+1.0))-0.5);
	for(int i=0;i<MaxRaySteps;i++){
		if(t>lightDist || s<0.001) break;
		float r=rCoC+t*coneGrad;//radius of cone
#ifndef PERFECT_DE
		//Because most of the time (in particular with fractals) DE is not well defined inside. The soft shadow doesn't "bleed" inside.
		d=DEF2(ro+rd*(t+r*jitter));
		s*=linstep(-r,r,d);//smoothstep(-r,r,d);//
		t+=abs(0.75*d+.25*r);//*mix(1.,0.2*rand(gl_FragCoord.xy*0.02*vec2(i)*t),jitter);
#else
		//in the case the DE gives correct interior distance this is the "correct" formula giving "correct" shadow.
		d=DEF2(ro+rd*(t+r*jitter))+r;
		s*=linstep(0.,2.*r,1.*d);//smoothstep(-r,r,d);//
		t+=abs(0.75*d+.25*r);
#endif
	}
	s=max(0.,s-0.001)/0.999;
	return clamp(1.-s,0.0,1.0);
}
#else
// Uses the soft-shadow approach by Quilez:
// http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
float shadow(vec3 ro, vec3 lightPos, float eps) {
		vec3 rd=( lightPos-ro);
		float lightDist=length(rd);
		rd/=lightDist;
		float coneGrad=ShadowSoft;
		if(ShadowSoft==0.) coneGrad=lightDist/(LightSize+0.001); else coneGrad=ShadowSoft;
		float totalDist =2.0*eps;
		float s = 1.0; // where 1.0 means no shadow!
		float jitter=Dither*(rand(ro.xy)-0.5);
 		for (int steps=0; steps<MaxRaySteps && totalDist<lightDist; steps++) {
			vec3 p = ro + totalDist*(1.+jitter/coneGrad) * rd;
			float dist = DEF2(p);
			if (dist < eps)  return 1.0;
			s = min(s, coneGrad*(dist/totalDist));
			totalDist += dist;
		}
		return 1.0-s;
}
#endif

#if 1
// Ambient occlusion approximation.
// Sample proximity at a few points in the direction of the normal.
//tweaked to make it "multiscale".
float ambientOcclusion(vec3 p, vec3 n) {
	float ao = 0.0;
	float de = DEF(p)/aoEps;//*(-DetailAO);//dividing by aoEps gives control on the overall scale of the AO.
	float wSum = 0.0;
	float w = 1.0;
	float d = 1.0-(Dither*rand(p.xy));
	float D=1.;
	for (float i =1.0; i <15.; i++) {
		// D is the distance estimate difference.
		// If we move 'n' units in the normal direction,
		// we would expect the DE difference to be 'n' larger -
		// unless there is some obstructing geometry in place
		float prevD=D;
		D = (DEF(p+ d*n*de) -0.*de)/(d*de);
		D=min(prevD,D);//if light is obscured at previous level it should be obscured at least at the same amount at this level.
		w *= 1.;//Maybe removed next time
		d*=1.5;//next level  -> x2. maybe becomes a parameter
		ao += w*clamp(1.0-D,0.0,1.0);
		wSum += w;
	}
	return clamp(AO.w*ao/wSum, 0.0, 1.0);
}
#else
// Ambient occlusion approximation.
// Sample proximity at a few points in the direction of the normal.
float ambientOcclusion(vec3 p, vec3 n) {
	float ao = 0.0;
	float de = DEF(p);
	float wSum = 0.0;
	float w = 1.0;
	float d = 1.0-(Dither*rand(p.xy));
	for (float i =1.0; i <10.0; i++) {
		// D is the distance estimate difference.
		// If we move 'n' units in the normal direction,
		// we would expect the DE difference to be 'n' larger -
		// unless there is some obstructing geometry in place
		float D = (DEF(p+ d*n*aoEps) -de)/(d*aoEps);
		w *= 1.;
		d*=2.;
		ao += w*clamp(1.0-D,0.0,1.0);
		wSum += w;
	}
	return clamp(AO.w*ao/wSum, 0.0, 1.0);
}
#endif

vec3 lighting(vec3 n, vec3 color, vec3 pos, vec3 dir, float eps, out float shadowStrength, float ao) {
	shadowStrength = 0.0;
	//vec3 spotDir = vec3(sin(SpotLightDir.x*3.1415)*cos(SpotLightDir.y*3.1415/2.0), sin(SpotLightDir.y*3.1415/2.0)*sin(SpotLightDir.x*3.1415), cos(SpotLightDir.x*3.1415));
	//vec3 spotDir = -lightnormal(pos,eps);
	float D2L2=dot(LightPos-pos,LightPos-pos);
	float falloff=pow(D2L2,-LightFallOff);
	vec3 spotDir = normalize(LightPos-pos);

	float nDotL = max(AoCorrect,dot(n,spotDir));
	vec3 halfVector = normalize(-dir+spotDir);
	float diffuse = nDotL;
	float ambient = max(CamLightMin,dot(-n, dir));
	float hDotN = max(0.,dot(n,halfVector));

	// An attempt at Physcical Based Specular Shading:
	// http://renderwonk.com/publications/s2010-shading-course/
	// (Blinn-Phong with Schickl term and physical normalization)
	//float specular =((SpecularExp+2.)/8.)*pow(hDotN,SpecularExp+0.00001)*(SpecularExp + (1.-SpecularExp)*pow(1.-hDotN,5.))*nDotL*Specular;
	float f0=(SpecularExp-1.)/(SpecularExp+1.); f0*=f0;
	float fresnel=f0+(1.-f0)*pow(1.+dot(n,dir),5.);
	float specular =((SpecularExp+2.)/8.)*fresnel*nDotL*pow(hDotN,SpecularExp+0.00001)*Specular;
	//specular = min(SpecularMax,specular);

	if (HardShadow>0.0) {
		// check path from pos to spotDir
		shadowStrength = shadow(pos+n*eps, LightPos, eps);
		/*float shS= shadow(pos+n*eps, LightPos, 0.1);// attempt for SSS effect. Yes it is possible if the DE is quasi perfect (accurate and defined inside).
		shS=pow(shS,.5);
		shadowStrength = mix(shadowStrength, shS,0.5);*/
		//ambient = mix(ambient,0.0,0.0);
		diffuse = mix(diffuse,0.0,HardShadow*shadowStrength);
		// specular = mix(specular,0.0,HardShadow*f);
		specular*=1.-shadowStrength;//if (shadowStrength>0.0) specular = 0.0; // always turn off specular, if blocked
	}
	vec3 FG=fogAmount3(pos,LightPos);//fog Attenuates light.
	return (FG*SpotLight.xyz*SpotLight.w*falloff*(diffuse+ specular)+CamLight.xyz*CamLight.w*(ambient)*(1.-ao))*color;
}

vec3 colorBase = vec3(0.0,0.0,0.0);

vec3 cycle(vec3 c, float s) {
	return vec3(0.5)+0.5*vec3(cos(s*Cycles+c.x),cos(s*Cycles+c.y),cos(s*Cycles+c.z));
}


#group Palette

uniform bool paletteColoring; checkbox[true]
uniform vec3 pBaseColor;color[0,0,0]
uniform float BaseStrength;slider[0,.3,1]
//speed of palette cycling
uniform float cSpeed; slider[0,10.,50.00]
//palette offset...  rotates pallete so you can put colors where you want 'em
uniform float pOffset; slider[0,0,100]
uniform vec3 color0;color[0.95,0.83,0.42]
uniform bool Sharp0to1;checkbox[false]
uniform float Dist0to1;slider[0,1,3]
uniform vec3 color1;color[1.,0.,0.07]
uniform bool Sharp1to2;checkbox[false]
uniform float Dist1to2;slider[0,1,3]
uniform vec3 color2;color[.7,.7,0.42]
uniform bool Sharp2to3;checkbox[false]
uniform float Dist2to3;slider[0,1,3]
uniform vec3 color3;color[1.,0.37,0.]
uniform bool Sharp3to0;checkbox[false]
uniform float Dist3to0;slider[0,1,3]

//uniform bool Orbit; checkbox[False]
uniform vec4 orbitStrengthXYZR;slider[(-3,-3,-3,-3),(1,1,1,1),(3,3,3,3)]

float PaletteCycleDistance=
			(Dist0to1+Dist1to2+Dist2to3+Dist3to0);
float dist01=Dist0to1/PaletteCycleDistance;
float dist12=Dist1to2/PaletteCycleDistance;
float dist23=Dist2to3/PaletteCycleDistance;
float dist30=Dist3to0/PaletteCycleDistance;
float cyclespeedadjusted=cSpeed*.1;
float poffset=pOffset/100.;

vec3 palette(vec4 p) {
	float orbittotal=orbitStrengthXYZR.x*orbitTrap.x+
				orbitStrengthXYZR.y*orbitTrap.y+
				orbitStrengthXYZR.z*orbitTrap.z+
				orbitStrengthXYZR.w*orbitTrap.w;

	orbittotal=mod(abs(orbittotal)*cyclespeedadjusted,1.);
	orbittotal=mod(orbittotal+poffset,1.);
	vec3 colormix;
	if (orbittotal<=dist01) {
		if (Sharp0to1) {
			colormix=mix(color0,pBaseColor,BaseStrength);
		} else {
			colormix=mix(color0,color1,abs(orbittotal)/(dist01));
			colormix=mix(colormix,pBaseColor,BaseStrength);
		}
	} else if (orbittotal<=dist01+dist12) {
		if (Sharp1to2) {
			colormix=mix(color1,pBaseColor,BaseStrength);
		} else {
			colormix=mix(color1,color2,abs(orbittotal-dist01)/abs(dist12));
			colormix=mix(colormix,pBaseColor,BaseStrength);
		}
	} else if (orbittotal<=dist01+dist12+dist23) {
		if (Sharp2to3) {
			colormix=mix(color2,pBaseColor,BaseStrength);
		} else {
			colormix=mix(color2,color3,abs(orbittotal-dist01-dist12)/abs(dist23));
			colormix=mix(colormix,pBaseColor,BaseStrength);
		}
	} else {
		if (Sharp3to0) {
			colormix=mix(color3,pBaseColor,BaseStrength);
		} else {
			colormix=mix(color3,color0,abs(orbittotal-dist01-dist12-dist23)/abs(dist30));
			colormix=mix(colormix,pBaseColor,BaseStrength);
		}
	}

	return colormix;
}

#group Texture
uniform bool ifTexture; checkbox[false]
//uniform sampler2D htex; file[../]
uniform sampler2D tex; file[texture.jpg]
uniform float TextSpeedMult; slider[.01,1.,20.]
uniform float TextureSpeed; slider[-5.,1.,5.]
float texturespeed=TextureSpeed*TextSpeedMult;
uniform float intensity; slider[-6,2.5,6.]
uniform vec4 orbitTexX;slider[(-1,-1,-1,-1),(0,0,0,0),(1,1,1,1)]
uniform vec4 orbitTexY;slider[(-1,-1,-1,-1),(1,0,0,0),(1,1,1,1)]


uniform vec2 TextureOffset; slider[(-100,-100),(0,0),(100,100)]
vec2 textoff=TextureOffset/100.0;
uniform int MapType; slider[0,1,10]
//uniform int ColorIterations;  slider[0,8,100]
//trapmode for images
///uniform bool trapmode; checkbox[false]
uniform bool testA;checkbox[false]

 vec4 color4 = texture2D(tex,textoff)*intensity;
uniform float tsides; slider[2.,4.,30.]
uniform float tsides2; slider[2.,4.,30.]
void polymaptexture2 ( vec3 z,  vec2 hangles) {
	float rCyz=abs(atan(z.z,z.y));
	 float i=1.;
	while (rCyz>3.14159265359/tsides && i<tsides)
			 {rCyz-=6.28318530718/tsides; i++;}

		rCyz=.5*(rCyz/3.14159265359*tsides+1.);
	i=1.;
	 float rCxyz= abs(atan(sqrt(z.y*z.y+z.z*z.z),z.x));
		i=1.;
	while (rCxyz>3.14159265359/tsides2 && i<tsides2)
			 {rCxyz-=6.28318530718/tsides2; i++;}
	rCxyz=.5*(rCxyz/3.14159265359*tsides2+1.);
		hangles=vec2(rCyz,rCxyz);

}


vec3 TextureIT (vec4 orbitTrap) {
		vec2 orbittotal;
 	 vec4 color3=vec4(0.,0.,0.,0.);
		vec2 angles;
	// idea for maptype 0 from:
// https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Textured_Spheres
	if (MapType==0) {
		angles= vec2((atan(orbitTrap.z, orbitTrap.x) / 3.1415926 + 1.0) * 0.5,
                                  (asin(orbitTrap.y) / 3.1415926 + 0.5));
		color3=texture2D(tex,textoff+(angles)*texturespeed)*intensity;
	}    else if (MapType==1) {
		polymaptexture2(orbitTrap.xyz,angles);
		color3 = texture2D(tex,textoff+angles*texturespeed)*intensity;
	} 	else if (MapType==2) {
			orbittotal=vec2(orbitTexX.x*orbitTrap.x+
				orbitTexX.y*orbitTrap.y+
				orbitTexX.z*orbitTrap.z+
				orbitTexX.w*orbitTrap.w,
				orbitTexY.x*orbitTrap.x+
				orbitTexY.y*orbitTrap.y+
				orbitTexY.z*orbitTrap.z+
				orbitTexY.w*orbitTrap.w
				);
		orbittotal=((textoff+(orbittotal)*texturespeed));


		 color3 = texture2D(tex,orbittotal)*intensity;
	}  	else if (MapType==3) {
			orbittotal=vec2(orbitTexX.x*orbitTrap.x+
				orbitTexX.y*orbitTrap.y+
				orbitTexX.z*orbitTrap.z+
				orbitTexX.w*orbitTrap.w,
				orbitTexY.x*orbitTrap.x+
				orbitTexY.y*orbitTrap.y+
				orbitTexY.z*orbitTrap.z+
				orbitTexY.w*orbitTrap.w
				);
		orbittotal=((textoff+(orbittotal)*texturespeed));


		 color3 = texture2D(tex,orbittotal)*intensity;
	} else if (MapType==4) {
  		angles= vec2((atan(orbitTrap.y, orbitTrap.z) / 3.1415926 + 1.0) * 0.5,
                                  (atan(orbitTrap.x,length(orbitTrap.yz)) / 3.1415926 + 0.5));
		color3=texture2D(tex,textoff+(angles)*texturespeed)*intensity;
	} else if (MapType==5) {
   	color3 = texture2D(tex,textoff+ (orbitTrap.xz)*texturespeed)*intensity;
	} else if (MapType==6) {
   	color3 = texture2D(tex,textoff+ (orbitTrap.xy)*texturespeed)*intensity;
	} else if (MapType==7) {
   	color3 = texture2D(tex, textoff+(orbitTrap.yz)*texturespeed)*intensity;
	} else  if (MapType==8) {
 		color3 = texture2DProj (tex, orbitTrap*texturespeed)*intensity;
	} else if (MapType==9) {
		color3 = texture2D(tex,Dir.xy*texturespeed)*intensity;
	} else if (MapType==10) {
 		color3 = texture2D(tex, coord*texturespeed)*intensity;
	}
	return color3.xyz;
}


vec3 getColor() {
	vec3 orbitColor;
	vec3 color2;
	if (ifTexture) {
			color2= TextureIT(orbitTrap);
	} else if (paletteColoring) {
			color2 = palette(orbitTrap);
	} else {
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
			color2 = mix(BaseColor, 3.0*orbitColor,  OrbitStrength);
	}
	return color2;
}


#ifdef  providesColor
vec3 color(vec3 point, vec3 normal);
#endif

#group ColorSample

uniform int ThetaSteps; slider[2,7,30]
uniform int PhiSteps; slider[2,7,30]

//Intensity of color during calculation
uniform float ConeWidth;slider[0.01,6.283185307178,6.283185307178]
uniform float InteriorIntensity; slider[0,1.,10.]
//Weight of color mix during calculation
uniform float InteriorWeight; slider[0.01,.50,.99]

//weight to apply AFTER calculation
uniform float PostWeight; slider[0.,0.,1.]
//increase or decrease intensity of color after calculation
uniform float PostIntensity; slider[0,1.,10.]

uniform bool allsample;checkbox[false]
//uniform bool correctreflect;checkbox[false]

uniform bool normalColor; checkbox[true]
uniform float colorDepth; slider[-1.99,.0,1.99]


uniform int SampleStepsShort; slider[0,1,6]
uniform float SampleDepthShort; slider[-1.99,.0,1.99]
uniform bool LongFirst;checkbox[false]
uniform int SampleStepsLong; slider[0,0,6]
uniform float SampleDepthLong; slider[-1.99,0.,1.99]

bool orbitBoolSet;



bool lighthit=false;

vec3 trace(inout SRay Ray, inout vec3 hitNormal, inout float glow) {
	glow=1000.0;
	vec3 hit = SRCurrentPt(Ray); //from+dir*totalDist;
	orbitTrap = vec4(10000.0);
	vec3 direction = normalize(Dir);
	floorHit = false;
	floorDist = 0.0;
	vec3 direction2;

	int samplestepsL;
	int samplestepsS;
	int tsteps=1;
	int psteps;

	vec3 hitglow;
	vec3 hitglowa;
	bool flip=true;
	float theta;
	float phi;
	vec2 thetaphi;
	float sampledepth;

	float dist = 0.0;
	//float totalDist = 0.0;

	int steps;
	colorBase = vec3(0.0,0.0,0.0);

	// We will adjust the minimum distance based on the current zoom
	float eps = minDist;
	float lightde=0.;
	float epsModified = max(MIN_EPS, Ray.Pos*eps* FudgeFactor);
	orbitTrap = vec4(10000.0);
	vec3 p = SRCurrentPt(Ray);
	float ldist = min(DEF(p) * FudgeFactor, DElight(p));
	ldist *= (Dither*rand(Ray.Direction.xy))+(1.0-Dither);
	SRAdvance(Ray, ldist);
	for (steps=0; steps<MaxRaySteps; steps++) {
		vec3 p = SRCurrentPt(Ray); //from + totalDist * direction;
		//dist = DEF(p) * FudgeFactor;
		dist = lightde = DElight(p);//DEL(p, lightde);
		dist = min(dist,DEF(p) * FudgeFactor);
		glow=min(lightde,glow);
		SRAdvance(Ray, dist);
		epsModified = max(MIN_EPS, Ray.Pos*eps* FudgeFactor);
		if (dist < epsModified) {
			// move back
			SRAdvance(Ray, dist-2.5*epsModified); //totalDist -= (epsModified-dist);
			//float l = ldist*(epsModified-dist)/(ldist-1.*dist); SRAdvance(Ray, -ldist);
			break;
		}
		if (Ray.Pos > MaxDistance) {
			fSteps -= (Ray.Pos-MaxDistance)/dist;
			break;
		}
		//ldist = dist;
	}
	//if(dist==glow) lighthit=true;
	if (EnableFloor && dist ==floorDist*FudgeFactor) floorHit = true;
	vec3 hitColor=vec3(0.);
	float stepFactor = clamp((fSteps)/float(GlowMax),0.0,1.0);
	vec3 backColor = BackgroundColor;
	if (GradientBackground>0.0) {
		float t = length(coord);
		backColor = mix(backColor, vec3(0.0,0.0,0.0), t*GradientBackground);
	}

	if (  steps==MaxRaySteps) orbitTrap = vec4(0.0);
	vec3 colorhit;
	float shadowStrength = 0.0;
	if ( dist < epsModified) {
		if (dist==lightde) {lighthit=true; return SpotLight.xyz*SpotLight.w/(LightSize+0.01);}
		// We hit something, or reached MaxRaySteps
		hit = SRCurrentPt(Ray); //from + totalDist * direction;
		float ao = AO.w*stepFactor ;

		if (floorHit) {
			hitNormal = floorNormal;
			if(dot(floorNormal,p)-FloorHeight<0.) //if (dot(hitNormal,Ray.Direction)>0.0)
				hitNormal *=-1.0;
		} else {
			hitNormal= normal(hit-NormalBackStep*epsModified*Ray.Direction, epsModified); // /*normalE*epsModified/eps*/
		}
		if (steps!=MaxRaySteps) {   // && (colorDepth!=0 || normalColor)
			orbitBoolSet=true;
			colorhit= hit+colorDepth*direction;
			if (normalColor) {
				//vec3 colorhitNormal= normal(hit-NormalBackStep*epsModified*direction, epsModified);
				colorhit= hit+colorDepth*hitNormal;
			}
			DE(colorhit);
		}
#ifdef  providesColor
		hitColor = mix(BaseColor,  baseColor(hit,hitNormal),  OrbitStrength);
#else
		hitColor = getColor();
#endif
		hitglowa=hitColor;
			if (steps!=MaxRaySteps) {
				//	if (correctreflect) {
						//leave for now, fix later
						direction2=direction;
				//		theta=acos(direction2.x)+ConeWidth*.5;
				//		phi=atan(direction2.z,direction2.y)+ConeWidth*.5;
				//	} else {
							//normal... probably fix this stuff later
						theta=acos(hitNormal.x)+ConeWidth*.5;   //+pi/2=1.57079632679
						phi=atan(hitNormal.z,hitNormal.y)+ConeWidth*.5;
				//	}
					thetaphi=vec2 (theta,phi);
	if (LongFirst) {sampledepth=SampleDepthLong; }
				else 	 {sampledepth=SampleDepthShort;}
	for (samplestepsL=0; samplestepsL<=SampleStepsLong; samplestepsL++) {
		theta=thetaphi.x; phi=thetaphi.y;
		for (samplestepsS=0; samplestepsS<=SampleStepsShort; samplestepsS++) {
			theta=thetaphi.x; phi=thetaphi.y;
				tsteps=1;
				while (tsteps<ThetaSteps) {
					psteps=1;
						while (psteps<PhiSteps) {
							direction2=vec3(cos(theta),sin(theta)*cos(phi),sin(theta)*sin(phi));

								//we're just going to sample from points around the hit
								// and not bother to check for hits... way faster!
								colorhit= hit+sampledepth*direction2;
								orbitTrap = vec4(10000.0);
								orbitBoolSet=true;
								DE(colorhit);
								hitglow=getColor();
								if (allsample && flip) {
									hitglowa=hitglow;
									flip=false;
								} else {
									hitglowa=mix(abs(hitglowa),abs(hitglow)*InteriorIntensity,InteriorWeight);  //
								}
							phi=phi-ConeWidth/float(PhiSteps);
							psteps++;
						}  //end phi loop
					phi=thetaphi.y;
					theta=theta-ConeWidth/float(ThetaSteps);
					tsteps++;
				}  //end theta loop
			sampledepth+=SampleDepthShort;
		}  //end of sample short
		sampledepth+=SampleDepthLong;
	} //end of sample long
}   //end of sampling
		hitColor =mix(hitColor,hitglowa,PostWeight)*PostIntensity;

#ifndef linearGamma
		hitColor = pow(clamp(hitColor,0.0,1.0),vec3(Gamma));
#endif
		if (DetailAO<0.0) ao = ambientOcclusion(hit, hitNormal);
		if (floorHit) {
			hitColor = FloorColor;
		}

		//hitColor = mix(hitColor, AO.xyz ,ao);
		hitColor = lighting(hitNormal, hitColor,  hit,  Ray.Direction,epsModified,shadowStrength,ao);
		/*
		// OpenGL  GL_EXP2 like fog
		float f = Ray.Pos;//totalDist;
		//hitColor = mix(hitColor, backColor, 1.0-exp(-pow(Fog,4.0)*f*f));
		hitColor = mix(hitColor, backColor, 1.0-exp(-pow(Fog/FogExp,4.0)*pow(f*f,FogExp)));
		if (floorHit && false) {
			hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		}*/
	}
	else {
#ifdef providesBackground
		hitColor = backgroundColor(Ray.Direction); //dir);
#else
		hitColor = backColor;
#endif
		//hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		hitNormal = vec3(0.0);
		Ray.Pos=MaxDistance;//1000.;//if nothing hit return a big value. This is necesary for the fog.
	}
	if(depthFlag) {
		// do depth on the first hit not on reflections
		depthFlag=false;
		// for rendering depth to alpha channel in EXR images
		// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
		if(DepthToAlpha==true) gl_FragDepth = 1.0/Ray.Pos;
		else
		// sets depth for spline path occlusion
		// see http://www.fractalforums.com/index.php?topic=16405.0
		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
		clamp(Ray.Pos, 0.00001, 1000.0));
	}

	return hitColor;
}

#ifdef USE_IQ_CLOUDS
#group IQ_Clouds
//how large should clouds appear (larger clouds render faster)
uniform float CloudScale; slider[0.1,1.0,10.0]
//how flat are they
uniform float CloudFlatness; slider[0.0,0.0,0.9]
//how high do the clouds go
uniform float CloudTops; slider[-10.0,1.0,10.0]
//and how low
uniform float CloudBase; slider[-10.0,-1.0,10.0]
//how thick
uniform float CloudDensity; slider[0.0,1.0,1.0]
//are they smooth or rough
uniform float CloudRoughness; slider[0.0,1.0,2.0]
//how much light contrast do they create
uniform float CloudContrast; slider[0.0,1.0,10.0]
//what is the base color
uniform vec3 CloudColor; color[0.65,0.68,0.7]
//and the color of light hitting them (posLight doesn't have a color??)
uniform vec3 SunLightColor; color[0.7,0.5,0.3]
//wind direction
uniform vec3 WindDir; slider[(-1.0,-1.0,-1.0),(0.0,0.0,1.0),(1.0,1.0,1.0)]
//wind speed
uniform float WindSpeed; slider[0.0,1.0,2.0]

// float rand(vec3 co){// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
// 	return fract(sin(dot(co*0.123,vec3(12.9898,78.233,112.166))) * 43758.5453);
// }

float cnoyz(vec3 co){
	vec3 d=smoothstep(0.0,1.0,fract(co));
	co=floor(co);
	const vec2 v=vec2(1.0,0.0);
	return mix(mix(mix(rand(co),rand(co+v.xyy),d.x),
		mix(rand(co+v.yxy),rand(co+v.xxy),d.x),
		d.y),mix(mix(rand(co+v.yyx),rand(co+v.xyx),d.x),
		mix(rand(co+v.yxx),rand(co+v.xxx),d.x),d.y),d.z);
}

float cloudDensity( in vec3 p , in vec3 cloudDir, in float t)
{
	vec3 q=p/CloudScale;
	q/=vec3(1.0)-cloudDir*CloudFlatness;
	float f=0.0,a=0.5;
	for(int i=0;i<5;i++){
    		f+= a*cnoyz( q );
		q = q*2.03;
		a = a*0.5;
		if(i>6-int(5.0*t/MaxDistance))break;
	}
	float y=dot(p,cloudDir);
	float cDen=1.0 - 2.0*abs(0.5*(CloudTops+CloudBase)-y)/(CloudTops-CloudBase);
	return clamp( CloudDensity * cDen - CloudRoughness*f, 0.0, 1.0 );
}

vec4 integrateClouds( in vec4 sum, in float dif, in float den, in float t )
{
    // lighting
    vec3 lin = CloudColor*1.3 + SunLightColor*dif*CloudContrast;
    vec4 col = vec4( mix( 1.15*vec3(1.0,0.95,0.8), vec3(0.0), den ), den );
    col.xyz *= lin;
	col=clamp(col,0.0,1.0);
   // col.xyz = mix( col.xyz, bgcol, 1.0-exp(-0.0025*t*t) );
    // front to back blending
    col.a *= mix(0.9,0.0,clamp(t*t/(MaxDistance*MaxDistance),0.0,1.0));
    col.rgb *= col.a;
    return sum + col*(1.0-sum.a);
}

vec4 clouds(vec3 p0, vec3 p1){
	vec3 ro=p0 + (WindSpeed*(WindDir*time)),rd=normalize(p1-p0),cloudDir=normalize(HF_Dir);
	vec4 sum = vec4(0.0);
	float t=0.1*CloudScale*rand(vec3(gl_FragCoord.xyy+float(subframe)*30.0)),maxT=length(p1-p0);
	bool goingUp=(dot(rd,cloudDir)>0.0);
	while(t<maxT) {
		vec3  pos = ro + t*rd;
		if((!goingUp && dot(pos,cloudDir)<CloudBase) || (goingUp && dot(pos,cloudDir)>CloudTops) || sum.a > 0.99) break;
		float den = cloudDensity(pos,cloudDir,t);
		if( den>0.01 ) {
			vec3 sundir=normalize(LightPos-pos);
			float dif = clamp((den - cloudDensity(pos+0.3*CloudScale*sundir,cloudDir,t))/0.6, -1.0, 1.0 );
			sum = integrateClouds( sum, dif, den, t );
		}
		t += 0.1*CloudScale+0.02*CloudScale*t;
	}
	return clamp(sum,0.0,1.0);
}
#endif

#ifdef KN_VOLUMETRIC
float length2(vec3 p){ return dot(p,p);}
//See: https://www.shadertoy.com/view/Xdf3zB
//Good for relatively homogeneous media and for dark ambiance.
//This uses importance sampling wrt distance to light source. Looks good when anisotropy is small but noisy otherwise.
//Should use multipe importance sampling wrt:
// -distance to light (done)
// -Distance: we don't need details far away. (I don't think this is necessary in this particular case)
// -Extinction: when fog density is high enought.
// -Anisotropy: I gess this is as important as distance to light because somehow it have the inverse effect.
//ToDo2: Take fog color into account
//ToDo3: correct the use of pseudo-random number generator. (any help?)
int fogstrata=0;
vec3 ptLightGlow1(vec3 P0, vec3 P1){
	//Values used for importance sampling
	float A=length2(P0-LightPos);
	float B=dot(P1-P0, P0-LightPos);
	float C=length2(P1-P0);
	float Delta=sqrt(A*C-B*B);
	//get sample point using importance sampling.
	float x=(rand(viewCoord+vec2(1.6183+float(subframe+fogstrata)))+float(fogstrata))*1./float(HF_FogIter); fogstrata++;
	float atanB=atan(B/Delta), atanBC=atan((B+C)/Delta);//B+C=dot((P1-P0)*(P1-LightPos))
	float PDF_NF=(atanBC-atanB)/Delta;//Normalization factor due to division by the PDF.
	float t=(tan(mix(atanB,atanBC,x))*Delta-B)/C;//when x is close to 0. there are black dots!
	t=clamp(t,0.00001,1.); //This is very strange: I have to clamp this way to get rid from black dots. When doing clamp(t,0.,1.) I get much more black dots.
	vec3 Pt=P0+t*(P1-P0);//this is the sample point
	float extP0t=fogAmount(P0,Pt);//Transmittance between "eye" to sample
	float extLt=fogAmount(Pt,LightPos);//Transmittance between sample and light source.
	//Compute fog density at sample (redundent with fogAmount(). Should do something about it)
	vec3 hfdir=normalize(HF_Dir);
	float Ty=HF_Fallof*(dot(Pt,hfdir)-HF_Offset);
	float densiTy=HF_Intensity*exp1(-Ty)+HF_Const; //densiTy=clamp(densiTy,0.,100.);
	//Shadow
	float shadowStrength=1.;
	if(HF_CastShadow)
		shadowStrength = 1.-shadow(Pt, LightPos, 0.001);
	//Anisotopy factor
	float cosTheta=dot(normalize(Pt-P0),normalize(LightPos-Pt));//cos(theta): the angle between light direction and view direction at the sample
	float denom = 1. + HF_Anisotropy.x*HF_Anisotropy.x - 2.*HF_Anisotropy.x*cosTheta;
	float Anie=(1.-HF_Anisotropy.x*HF_Anisotropy.x)/sqrt(denom*denom*denom);//Dropped the 1./(4.*PI)* factor. Well This renderer is not meant to be physically accurate after all. :o)
	//Scattering at sample point
	float samVal=extP0t*extLt*densiTy*Anie*shadowStrength*sqrt(C);//sqrt(C) is the length of the path.
	//Return scattered light
	//samVal=max(0.,samVal);
	return SpotLight.xyz*SpotLight.w*PDF_NF*samVal*HF_Scatter;
}

vec3 ptLightGlow3(vec3 P0, vec3 P1){
	//Values used for importance sampling
	float A=length2(P0-LightPos);
	float B=dot(P1-P0, P0-LightPos);
	float C=length2(P1-P0);
	float Delta=sqrt(A*C-B*B);
	//get sample point using importance sampling.
	float x=(rand(viewCoord+vec2(1.6183+float(subframe+fogstrata)))+float(fogstrata))*1./float(HF_FogIter); fogstrata++;
	float atanB=atan(B/Delta), atanBC=atan((B+C)/Delta);//B+C=dot((P1-P0)*(P1-LightPos))
	float PDF_NF=(atanBC-atanB)/Delta;//Normalization factor due to division by the PDF.
	float t=(tan(mix(atanB,atanBC,x))*Delta-B)/C;//when x is close to 0. there are black dots!
	t=clamp(t,0.00001,1.); //This is very strange: I have to clamp this way to get rid from black dots. When doing clamp(t,0.,1.) I get much more black dots.
	vec3 Pt=P0+t*(P1-P0);//this is the sample point
	vec3 extP0t=fogAmount3(P0,Pt);//Transmittance between "eye" to sample
	vec3 extLt=fogAmount3(Pt,LightPos);//Transmittance between sample and light source.
	//Compute fog density at sample (redundent with fogAmount(). Should do something about it)
	vec3 hfdir=normalize(HF_Dir);
	float Ty=HF_Fallof*(dot(Pt,hfdir)-HF_Offset);
	vec3 densiTy=HF_Color.rgb*HF_Intensity*exp1(-Ty)+HF_Const; //densiTy=clamp(densiTy,0.,100.);
	//Shadow
	float shadowStrength=1.;
	if(HF_CastShadow)
		shadowStrength = 1.-shadow(Pt, LightPos, 0.001);
	//Anisotopy factor
	float cosTheta=dot(normalize(Pt-P0),normalize(LightPos-Pt));//cos(theta): the angle between light direction and view direction at the sample
	vec3 denom = 1. + HF_Anisotropy*HF_Anisotropy - 2.*HF_Anisotropy*cosTheta;
	vec3 Anie=(1.-HF_Anisotropy*HF_Anisotropy)/sqrt(denom*denom*denom);//Dropped the 1./(4.*PI)* factor. Well This renderer is not meant to be physically accurate after all. :o)
	//Scattering at sample point
	vec3 samVal=extP0t*extLt*densiTy*Anie*shadowStrength*sqrt(C);//sqrt(C) is the length of the path.
	//Return scattered light
	//samVal=max(0.,samVal);
	return SpotLight.xyz*SpotLight.w*PDF_NF*samVal*HF_Scatter;
}
#endif

vec3 color(SRay Ray) {
	float glow=0.5;
	vec3 hitNormal = vec3(0.0);
	vec3 col = vec3(0.0);
	vec3 RWeight=vec3(1.);

        depthFlag=true; // do depth on the first hit not on reflections

	for(int i=0; i<=ReflectionsNumber;i++){
		vec3 prevPos= SRCurrentPt(Ray);
		vec3 col0 = trace(Ray, hitNormal, glow);
		vec3 curPos= SRCurrentPt(Ray);
		vec3 FG = fogAmount3(prevPos, curPos);//get fog amount
		col0=mix(HF_Color.rgb*HF_Color.w,col0,FG);//modify color
		col0+=ptLightGlow(max(0.,glow));
#ifdef KN_VOLUMETRIC
		if(HF_Scatter*(HF_Intensity+HF_Const)>0.){
			vec3 colfogglow=vec3(0.);
			for(int j=0;j<HF_FogIter;j++) colfogglow+=ptLightGlow3(prevPos, curPos);
			col0+=colfogglow*1./float(HF_FogIter);//ptLightGlow1(prevPos, curPos);
		}
#endif
#ifdef USE_IQ_CLOUDS
		vec4 clds=clouds(prevPos, curPos);
		col0=col0*(1.0-clds.w)+clds.rgb;
#endif
		col+=col0*RWeight;
		RWeight *= Reflection * FG;//modify current opacity
		if (hitNormal == vec3(0.0) || dot(RWeight,RWeight)<0.0001 || lighthit || Ray.Pos>=MaxDistance) {//nothing hit or light hit or reflected light is too small
			break;
		}

		Ray=SRReflect(Ray, hitNormal, 0.*minDist);//reflect the ray
	}
	return max(vec3(0.),col);//Sometimes col<BigNegativeValue  ->black dots. Why? I don't know :-/. Solved by Eiffie & Syntopia See: http://www.fractalforums.com/fragmentarium/updating-of-de-raytracer/msg81003/#msg81003 . I keep it just in case .
}
