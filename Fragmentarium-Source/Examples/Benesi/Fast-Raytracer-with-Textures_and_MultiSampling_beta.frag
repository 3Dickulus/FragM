#donotrun
#buffer RGBA32F

//
//					   #include "Fast-Raytracer-with-Textures_and_MultiSampling_beta.frag"
//

//  Set the filenames and directory paths for the textures you use the most often
// in this file.  That way, it automatically loads them when you do your initial
// build

#info Default Raytracer (by Syntopia)
#camera 3D

#vertex

#group Camera

// Field-of-view
uniform float FOV; slider[0,0.4,2.0] NotLockable
uniform vec3 Eye; slider[(-50,-50,-50),(0,0,-7),(50,50,50)] NotLockable
uniform vec3 Target; slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
uniform vec3 Up; slider[(0,0,0),(0,1,0),(0,0,0)] NotLockable
//uniform vec3 feedback;
//vec3 feedbackstorage=Eye;
//varying vec3 dirDx;
//varying vec3 dirDy;
varying vec3 from;
uniform vec2 pixelSize;
varying vec2 coord;
//varying float zoom;
varying vec3 dir;
//varying vec3 Dir;
void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
	coord.x*= pixelSize.y/pixelSize.x;
	// we will only use gl_ProjectionMatrix to scale and translate, so the following should be OK.
//	vec2 ps =vec2(pixelSize.x*gl_ProjectionMatrix[0][0], pixelSize.y*gl_ProjectionMatrix[1][1]);
//	zoom = length(ps);
	from=Eye;
	//if (feedback!=feedbackstorage) {
	//	from= vec3(feedback.x*pixelSize.x,feedback.y*pixelSize.y,feedback.z);
	//	feedbackstorage=feedback;
	//}
	vec3 Dir = normalize(Target-Eye);  //vec3
	vec3 up = Up-dot(Dir,Up)*Dir;
	up = normalize(up);
	vec3 Right = normalize( cross(Dir,up));
	dir = (coord.x*Right + coord.y*up )*FOV+Dir;
//	dirDy = ps.y*up*FOV;
//	dirDx = ps.x*Right*FOV;
}
#endvertex
///varying Eye;

#group Post
// Available when using exr image filename extention
uniform bool DepthToAlpha; checkbox[true];
// Magnitude
uniform float depthMag; slider[0.,1.,2.];
// Show the depth values as greyscale
uniform bool  ShowDepth; checkbox[false]
// for rendering depth to alpha channel in EXR images, set in DE-Raytracer.frag
// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
float depth = 1.0;
bool depthFlag = true; // do depth on the first hit not on reflections

#group Raytracer

// Camera position and target.
varying vec3 from,dir;  //,dirDx,dirDy;
varying vec2 coord;
//varying float zoom;

// HINT: for better results use Tile Renders and resize the image yourself
uniform int AntiAlias;slider[1,1,5] Locked
// Distance to object at which raymarching stops.
uniform float Detail;slider[-7,-3.0,0];
// The step size when sampling AO (set to 0 for old AO)
uniform float DetailAO;slider[-7,-0.5,0];

const float ClarityPower = 1.0;

// Lower this if the system is missing details
uniform float FudgeFactor;slider[0,.9,1];

float minDist = pow(10.0,Detail);
float aoEps = pow(10.0,DetailAO);
float MaxDistance = 100.0;

// Maximum number of  raymarching steps.
uniform int MaxRaySteps;  slider[0,111,2000]

// Used to speed up and improve calculation
uniform float BoundingSphere;slider[0,2,100];

// Can be used to remove banding
uniform float Dither;slider[0,0.5,1];

// Used to prevent normals from being evaluated inside objects.
uniform float NormalBackStep; slider[0,1,10] Locked

#group Light

// AO based on the number of raymarching steps
uniform vec4 AO; color[0,0.7,1,0.0,0.0,0.0];

// The specular intensity of the directional light
uniform float Specular; slider[0,4.0,10.0];
// The specular exponent
uniform float SpecularExp; slider[0,16.0,100.0];
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

vec4 orbitTrap = vec4(10000.0);
float fractionalCount = 0.0;

//#group Coloring
//uniform bool LegacyColor;checkbox[false]
// This is the pure color of object (in white light)
//uniform vec3 BaseColor; color[1.0,1.0,1.0];
// Determines the mix between pure light coloring and pure orbit trap coloring
//uniform float OrbitStrength; slider[0,0,1]

// Closest distance to YZ-plane during orbit
//uniform vec4 X; color[-1,0.7,1,0.5,0.6,0.6];

// Closest distance to XZ-plane during orbit
//uniform vec4 Y; color[-1,0.4,1,1.0,0.6,0.0];

// Closest distance to XY-plane during orbit
//uniform vec4 Z; color[-1,0.5,1,0.8,0.78,1.0];

// Closest distance to  origin during orbit
//uniform vec4 R; color[-1,0.12,1,0.4,0.7,1.0];


float DE(vec3 pos) ; // Must be implemented in other file

//uniform bool CycleColors; checkbox[false]
//uniform float Cycles; slider[0.1,1.1,32.3]

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

uniform bool EnableFloor; checkbox[false] Locked
uniform vec3 FloorNormal; slider[(-1,-1,-1),(0,0,0),(1,1,1)]
uniform float FloorHeight; slider[-5,0,5]
uniform vec3 FloorColor; color[1,1,1]


vec3 lighting(vec3 n, vec3 color, vec3 pos, vec3 dir, float eps, out float shadowStrength) {
	shadowStrength = 0.0;
	vec3 spotDir = vec3(sin(SpotLightDir.x*3.1415)*cos(SpotLightDir.y*3.1415/2.0), sin(SpotLightDir.y*3.1415/2.0)*sin(SpotLightDir.x*3.1415), cos(SpotLightDir.x*3.1415));
	spotDir = normalize(spotDir);
	// Calculate perfectly reflected light

	//if (dot(dir,n)>0.0) n = -n;

	vec3 r = spotDir - 2.0 * dot(n, spotDir) * n;

	float s = max(0.0,dot(dir,-r));


	float diffuse = max(0.0,dot(-n,spotDir))*SpotLight.w;
	float ambient = max(CamLightMin,dot(-n, dir))*CamLight.w;
	float specular = (SpecularExp<=0.0) ? 0.0 : pow(s,SpecularExp)*Specular;

	//if (dot(n,dir)<0.0) { specular = 0.0; }

	return (SpotLight.xyz*diffuse+CamLight.xyz*ambient+ specular*SpotLight.xyz)*color;
}

vec3 colorBase = vec3(0.0,0.0,0.0);

//vec3 cycle(vec3 c, float s) {
//	return vec3(0.5)+0.5*vec3(cos(s*Cycles+c.x),cos(s*Cycles+c.y),cos(s*Cycles+c.z));
//}

#group Palette
//normally use texture coloring, if you want to add some palette, increase palette percent
uniform float PalettePercent;slider[0,0,100]
float palpercent=PalettePercent/100.;

uniform vec3 pBaseColor;color[0,0,0]
uniform float BaseStrength;slider[0,.3,1]
//speed of palette cycling
uniform float cSpeed; slider[0,10.,50.00]
//palette offset...  rotates pallete so you can put colors where you want 'em
uniform float pOffset; slider[0,0,100]
uniform float pIntensity;slider[-4.,1.,4.]
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

	return colormix*pIntensity;
}



#group Textures

uniform int BaseColorTex;slider[1,1,4]
int ColorTex=BaseColorTex;
uniform int SampleColorTex;slider[1,1,4]
// Background color
uniform vec3 BackgroundColor; color[0.6,0.6,0.45]
// Vignette background
uniform float GradientBackground; slider[0.0,0.3,5.0]

uniform sampler2D tex1; file[texture.jpg]
uniform sampler2D tex2; file[texture2.jpg]
uniform sampler2D tex3; file[vigga.jpg]
uniform sampler2D tex4; file[Ditch-River_Env.hdr]

uniform float SpeedMult; slider[.01,1.,20.]
uniform float TextureSpeed; slider[-5.,1.,5.]

float texturespeed=TextureSpeed*SpeedMult;
uniform float intensity; slider[-6,2.5,6.]
uniform vec4 orbitTexX;slider[(-1,-1,-1,-1),(0,0,0,0),(1,1,1,1)]
uniform vec4 orbitTexY;slider[(-1,-1,-1,-1),(1,0,0,0),(1,1,1,1)]


uniform vec2 TextureOffset; slider[(-100,-100),(0,0),(100,100)]
vec2 textoff=TextureOffset/100.0;
uniform int BaseMapType; slider[0,2,5]
int MapType=BaseMapType;

 vec4 tcolor1 = texture2D(tex1,textoff);
 vec4 tcolor2 = texture2D(tex2,textoff);
 vec4 tcolor3 = texture2D(tex3,textoff);
 vec4 tcolor4 = texture2D(tex4,textoff);

uniform float tsides; slider[2.,4.,30.]
uniform float tsides2; slider[2.,4.,30.]
void polymaptexture2 (vec3 z, vec2 hangles) {
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
		vec2 orbittotal=vec2(0.,0.);
		vec2 angles;

// https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Textured_Spheres
	if (MapType==0) {
		angles= vec2((atan(orbitTrap.z, orbitTrap.x) / 3.1415926 + 1.0) * 0.5,
                                  (asin(orbitTrap.y) / 3.1415926 + 0.5));
		orbittotal=textoff+angles*texturespeed;
		//color=texture2D(tex,textoff+(angles)*texturespeed)*intensity;
	}    else if (MapType==1) {
		polymaptexture2(orbitTrap.xyz,angles);
		orbittotal=textoff+angles*texturespeed;
		//color = texture2D(tex,textoff+angles*texturespeed)*intensity;
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


		// color = texture2D(tex,orbittotal)*intensity;
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


		 //color = texture2D(tex,orbittotal)*intensity;
	} else if (MapType==4) {
  		angles= vec2((atan(orbitTrap.y, orbitTrap.z) / 3.1415926 + 1.0) * 0.5,
                                  (atan(orbitTrap.x,length(orbitTrap.yz)) / 3.1415926 + 0.5));
		orbittotal=textoff+angles*texturespeed;
			//color=texture2D(tex,textoff+(angles)*texturespeed)*intensity;
	} else if (MapType==5) {
		orbittotal=textoff+coord*texturespeed;
 		//color = texture2D(tex, coord*texturespeed)*intensity;
	}

	if (ColorTex==1) {
		tcolor1=texture2D(tex1,orbittotal)*intensity;
	} else if (ColorTex==2) {
		tcolor1=texture2D(tex2,orbittotal)*intensity;
	} else if (ColorTex==3) {
		tcolor1=texture2D(tex3,orbittotal)*intensity;
	} else if (ColorTex==4) {
		tcolor1=texture2D(tex4,orbittotal)*intensity;
	}

	return tcolor1.xyz;
}


vec3 getColor() {
	vec3 orbitColor;
	vec3 color;
			color= TextureIT(orbitTrap);
			orbitColor=   palette(orbitTrap);
			color=mix(color,orbitColor,palpercent);
	return color;
}


#group HeightMap
uniform int HeightMapType;slider[0,0,5]
uniform int HeightMap1Tex;slider[1,1,4]
uniform int HeightIter; slider[0,0,20]
uniform float HighStrength1; slider[-20,1,20]
uniform vec2 hTextureOffset1; slider[(-100,-100),(0,0),(100,100)]
uniform float hTextureSpeed1; slider[-5.,1.,5.]
float hintensity=HighStrength1*.02;




#ifdef  providesColor
vec3 color(vec3 point, vec3 normal);
#endif

#group ColorSample
uniform int SampleMapType;slider[0,2,5]
uniform float SampleSpeed; slider[-5.,1.,5.]
uniform vec2 SampleOffset; slider[(-100,-100),(0,0),(100,100)]
uniform int ThetaSteps; slider[2,5,30]
uniform int PhiSteps; slider[2,7,30]

//Intensity of color during calculation
uniform float ConeWidth;slider[0.01,6.283185307178,6.283185307178]
uniform float InteriorIntensity; slider[0,1.,10.]
//Weight of color mix during calculation
uniform float InteriorWeight; slider[0.01,.50,.99]

//weight to apply AFTER calculation
uniform float PostWeight; slider[0.,0.6,1.]
//increase or decrease intensity of color after calculation
uniform float PostIntensity; slider[0,1.,10.]

uniform bool allsample;checkbox[false]
//uniform bool correctreflect;checkbox[false]

uniform bool normalColor; checkbox[true]
uniform float colorDepth; slider[-1.99,.0,1.99]


uniform int SampleStepsShort; slider[0,3,6]
uniform float SampleDepthShort; slider[-1.99,.0002,1.99]
uniform bool LongFirst;checkbox[false]
uniform int SampleStepsLong; slider[0,2,6]
uniform float SampleDepthLong; slider[-1.99,0.002,1.99]

bool orbitBoolSet=false;

vec3 trace(vec3 from, vec3 dir, inout vec3 hit, inout vec3 hitNormal) {
	hit = vec3(0.0);
	orbitTrap = vec4(10000.0);
	vec3 direction = normalize(dir);
	vec3 direction2;
	float dist = 0.0;
	float totalDist = 0.0;

	int steps;
	int samplestepsL;
	int samplestepsS;
	int tsteps=1;
	int psteps;

	colorBase = vec3(0.0,0.0,0.0);
	vec3 hitglow;
	vec3 hitglowa;
	bool flip=true;
	float theta;
	float phi;
	vec2 thetaphi;
	float sampledepth;

	// We will adjust the minimum distance based on the current zoom
	float eps = minDist;
	float epsModified = 0.0;

	for (steps=0; steps<MaxRaySteps; steps++) {
		orbitTrap = vec4(10000.0);
		vec3 p = from + totalDist * direction;
			dist = DE(p);
		dist *= FudgeFactor;

		totalDist += dist;
		epsModified = pow(totalDist,ClarityPower)*eps;
		if (dist < epsModified) break;
		if (totalDist > MaxDistance) break;
	}

	vec3 hitColor;
	float stepFactor = clamp((float(steps))/float(GlowMax),0.0,1.0);
	vec3 backColor = BackgroundColor;
	if (GradientBackground>0.0) {
		float t = length(coord);
		backColor = mix(backColor, vec3(0.0,0.0,0.0), t*GradientBackground);
	}

	if (  steps==MaxRaySteps) orbitTrap = vec4(0.0);

	if ( dist < epsModified) {
		// We hit something, or reached MaxRaySteps
		hit = from + totalDist * direction;
		float ao = AO.w*stepFactor ;
		vec3 colorhit;
		hitNormal= normal(hit-NormalBackStep*epsModified*direction, epsModified); // /*normalE*epsModified/eps*/
		if (steps!=MaxRaySteps) {
			orbitBoolSet=true;
			colorhit= hit+colorDepth*direction;
			if (normalColor) {
				colorhit= hit+colorDepth*hitNormal;
			}
			DE(colorhit);
		}
		  //set colortex for base color texture
		#ifdef  providesColor
		hitColor = mix(BaseColor,  color(hit,hitNormal),  OrbitStrength);
		#else
			MapType=BaseMapType;
			 texturespeed=TextureSpeed*SpeedMult;
			 textoff=TextureOffset/100.0;
			ColorTex=BaseColorTex;
			hitColor = getColor();
		#endif
		MapType=SampleMapType;
		texturespeed=SampleSpeed*SpeedMult;
 		textoff=SampleOffset/100.0;
		ColorTex=SampleColorTex;   // set colortex to sample color texture
		hitglowa=hitColor;
		if (steps!=MaxRaySteps) {
			direction2=direction;
			theta=acos(hitNormal.x)+ConeWidth*.5;   //+pi/2=1.57079632679
			phi=atan(hitNormal.z,hitNormal.y)+ConeWidth*.5;
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
							colorhit= hit+sampledepth*direction2;
							orbitTrap = vec4(10000.0);
							orbitBoolSet=true;
							DE(colorhit);
							ColorTex=SampleColorTex;
							hitglow=getColor();
							hitglowa=mix(abs(hitglowa),abs(hitglow)*InteriorIntensity,InteriorWeight);
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
		hitColor = mix(hitColor, AO.xyz ,ao);
		float shadowStrength = 0.0;
		hitColor = lighting(hitNormal, hitColor,  hit,  direction,epsModified,shadowStrength);
				// OpenGL  GL_EXP2 like fog
		float f = totalDist;
		hitColor = mix(hitColor, backColor, 1.0-exp(-pow(Fog,4.0)*f*f));
	}	else {
		hitColor = backColor;
		hitColor +=Glow.xyz*stepFactor* Glow.w;
	}

	if(depthFlag) {
		// do depth on the first hit not on reflections
		depthFlag=false;
		// for rendering depth to alpha channel in EXR images
		// see http://www.fractalforums.com/index.php?topic=21759.msg87160#msg87160
		depth = 1.0/totalDist;
		if(DepthToAlpha==true) gl_FragDepth = depth;
		else
		// sets depth for spline path occlusion
		// see http://www.fractalforums.com/index.php?topic=16405.0
		gl_FragDepth = ((1000.0 / (1000.0 - 0.00001)) +
		(1000.0 * 0.00001 / (0.00001 - 1000.0)) /
		clamp(totalDist, 0.00001, 1000.0));
	}
	return hitColor;
}

#ifdef providesInit
void init(); // forward declare
#else
void init() {}
#endif

void main() {
	init();
	vec3 hitNormal = vec3(0.0);
	vec3 hit;
        depthFlag=true; // do depth on the first hit not on reflections
	vec3 color =  trace(from,dir,hit,hitNormal);
	if (ShowDepth) color = vec3(depthMag * depth);
	color = clamp(color, 0.0, 1.0);
	gl_FragColor = vec4(color, 1.0);
}


/*

oldmaps:
	if (MapType==1) {
			orbittotal=vec2(orbitTexX.x*orbitTrap.x+
				orbitTexX.y*orbitTrap.y+
				orbitTexX.z*orbitTrap.z+
				orbitTexX.w*orbitTrap.w,orbitTexY.x*orbitTrap.x+
				orbitTexY.y*orbitTrap.y+
				orbitTexY.z*orbitTrap.z+
				orbitTexY.w*orbitTrap.w
				);
		orbittotal=((textoff+(orbittotal)*texturespeed));
		if (testA) {
			orbittotal=vec2(mod(orbittotal.x,1.),mod(orbittotal.y,1.));
		}

		 color = texture2D(tex,orbittotal)*intensity;
	} else if (MapType==2) {
  		angles= vec2((atan(orbitTrap.y, orbitTrap.z) / 3.1415926 + 1.0) * 0.5,
                                  (atan(orbitTrap.x,length(orbitTrap.yz)) / 3.1415926 + 0.5));
		color=texture2D(tex,textoff+(angles)*texturespeed)*intensity;
	} else if (MapType==3) {
   	color = texture2D(tex,textoff+ (orbitTrap.xz)*texturespeed)*intensity;
	} else if (MapType==4) {
   	color = texture2D(tex,textoff+ (orbitTrap.xy)*texturespeed)*intensity;
	} else if (MapType==5) {
   	color = texture2D(tex, textoff+(orbitTrap.yz)*texturespeed)*intensity;
	} else if (MapType==7) {
  		color = texture2D(tex, coord*orbitTrap.w*texturespeed)*intensity;
	} else  if (MapType==6) {
 		color = texture2DProj (tex, orbitTexX*orbitTexY*orbitTrap*texturespeed)*intensity;
	} else if (MapType==8) {
		 color = texture2D(tex,dir.xy*texturespeed)*intensity;
	} else if (MapType==9) {
 		color = texture2D(tex, coord*texturespeed)*intensity;
	} else if (MapType==10) {
  		color = texture2DProj (tex, dir*texturespeed)*intensity;
	} else if (MapType==11) {
		 color = texture2D(tex,vec2(dir.x,length(dir.yz))*texturespeed)*intensity;
	} else if (MapType==12) {
		color = texture2D(tex,vec2(length(dir.xy),length(dir.xz))*texturespeed)*intensity;
	}
*/
