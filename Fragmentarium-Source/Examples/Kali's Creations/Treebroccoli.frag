// Output generated from file: C:/Fractales/Fragmentarium/surfkifs.frag
// Created: dom 5. feb 14:04:32 2012
// Output generated from file: C:/Fractales/Fragmentarium/Examples/SurfaceKIFS.frag
// Created: mi� 25. ene 22:59:36 2012
#info Mandelbox Distance Estimator (Rrrola's version).
//#donotrun
#buffer RGBA32F
#info Default Raytracer (by Syntopia)
#camera 3D

#vertex

#group Camera

// Field-of-view
uniform float FOV; slider[0,0.4,2.0] NotLockable
uniform vec3 Eye; slider[(-50,-50,-50),(0,0,-10),(50,50,50)] NotLockable
uniform vec3 Target; slider[(-50,-50,-50),(0,0,0),(50,50,50)] NotLockable
uniform vec3 Up; slider[(0,0,0),(0,1,0),(0,0,0)] NotLockable

varying vec3 dirDx;
varying vec3 dirDy;
varying vec3 from;
uniform vec2 pixelSize;
varying vec2 coord;
varying float zoom;
varying vec3 dir;
void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
	coord.x*= pixelSize.y/pixelSize.x;
	// we will only use gl_ProjectionMatrix to scale and translate, so the following should be OK.
	vec2 ps =vec2(pixelSize.x*gl_ProjectionMatrix[0][0], pixelSize.y*gl_ProjectionMatrix[1][1]);
	zoom = length(ps);
	from = Eye;
	vec3 Dir = normalize(Target-Eye);
	vec3 up = Up-dot(Dir,Up)*Dir;
	up = normalize(up);
	vec3 Right = normalize( cross(Dir,up));
	dir = (coord.x*Right + coord.y*up )*FOV+Dir;
	dirDy = ps.y*up*FOV;
	dirDx = ps.x*Right*FOV;
}
#endvertex

#group Raytracer

// Camera position and target.
varying vec3 from,dir,dirDx,dirDy;
varying vec2 coord;
varying float zoom;

// HINT: for better results use Tile Renders and resize the image yourself
uniform int AntiAlias;slider[1,1,5] Locked
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

uniform bool EnableFloor; checkbox[false] Locked
uniform vec3 FloorNormal; slider[(-1,-1,-1),(0,0,0),(1,1,1)]
uniform float FloorHeight; slider[-5,0,5]
uniform vec3 FloorColor; color[1,1,1]
bool floorHit = false;
float floorDist = 0.0;
vec3 floorNormal = normalize(FloorNormal);
int fSteps = 0;
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
 		for (int steps=0; steps<MaxRaySteps/10 && totalDist<MaxDistance; steps++) {
			vec3 p = pos + totalDist * sdir;
			float dist = DEF2(p);
			if (dist < eps)  return 1.0;
			s = min(s, ShadowSoft*(dist/totalDist));
			totalDist += dist;
		}
		return 1.0-s;	
}

float rand(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

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

       if (dot(n,dir)>0.0) { specular = 0.0; }   

	if (HardShadow>0.0) {
		// check path from pos to spotDir
		shadowStrength = shadow(pos+n*eps, -spotDir, eps);
		ambient = mix(ambient,0.0,HardShadow*shadowStrength);
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
	vec3 color(vec3 point, vec3 normal);
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
	
	// Check for bounding sphere
	float dotFF = dot(from,from);
	float d = 0.0;
	fSteps = 0;
	float dotDE = dot(direction,from);
	float sq =  dotDE*dotDE- dotFF + BoundingSphere*BoundingSphere;
	
	if (sq>0.0) {
		d = -dotDE - sqrt(sq);
		if (d<0.0) {
			// "minimum d" solution wrong direction
			d = -dotDE + sqrt(sq);
			if (d<0.0) {
				// both solution wrong direction
				sq = -1.0;
			} else {
				// inside sphere
				d = 0.0;
			}
		}
	}
	
	// We will adjust the minimum distance based on the current zoom
	float eps = minDist; // *zoom;//*( length(zoom)/0.01 );
	float epsModified = 0.0;
		
	if (sq<0.0) {
		// outside bounding sphere - and will never hit
		dist = MaxDistance;
		totalDist = MaxDistance;
		steps = 2;
	}  else {
		totalDist += d; // advance ray to bounding sphere intersection
		for (steps=0; steps<MaxRaySteps; steps++) {
			orbitTrap = vec4(10000.0);
			vec3 p = from + totalDist * direction;
			dist = DEF(p);
			//dist = clamp(dist, 0.0, MaxDistance)*FudgeFactor;
			dist *= FudgeFactor;

			if (steps == 0) dist*=(Dither*rand(direction.xy))+(1.0-Dither);
			totalDist += dist;
			epsModified = pow(totalDist,ClarityPower)*eps;
			if (dist < epsModified) break;
                    if (totalDist > MaxDistance) break;
		}
	}
	if (EnableFloor && dist ==floorDist*FudgeFactor) floorHit = true;
 	
	vec3 hitColor;
	float stepFactor = clamp((float(fSteps))/float(GlowMax),0.0,1.0);
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
		hitColor = mix(BaseColor,  color(hit,hitNormal),  OrbitStrength);
#else
		hitColor = getColor();
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
		if (floorHit) {
			hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
		}	
	}
	else {
		hitColor = backColor;
		hitColor +=Glow.xyz*stepFactor* Glow.w*(1.0-shadowStrength);
	
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
	
	vec3 color = vec3(0.0,0.0,0.0);
	for (int x = 0; x<AntiAlias; x++) {
		float  dx = float(x)/float(AntiAlias);
		for (int y = 0; y<AntiAlias; y++) {
			float dy = float(y)/float(AntiAlias);
			vec3 nDir = dir+dirDx*dx+dirDy*dy;
			vec3 hitNormal = vec3(0.0);
			vec3 hit;
			vec3 c = trace(from,nDir,hit,hitNormal);
			if (Reflection>0.0 && (hit != vec3(.0))) {
				vec3 d; vec3 d2 = vec3(0.0);
				// todo: minDist = modifiedEps?
				vec3 r = normalize(nDir - 2.0 * dot(hitNormal, nDir) * hitNormal);
	
				vec3 c2 = trace(hit+4.0*r*minDist,r,d,d2);
				color += c+c2*Reflection;
			} else {
				color += c;
			} 			

		}
	}
	
	color = clamp(color/float(AntiAlias*AntiAlias), 0.0, 1.0);
	gl_FragColor = vec4(color, 1.0);
}

#group default
#donotrun

// Standard matrices

// Return rotation matrix for rotating around vector v by angle
mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

mat3 rotationMatrixXYZ(vec3 v) {
	return rotationMatrix3(vec3(1.0,0.0,0.0), v.x)*
	rotationMatrix3(vec3(0.0,1.0,0.0), v.y)*
	rotationMatrix3(vec3(0.0,0.0,1.0), v.z);
}

// Return rotation matrix for rotating around vector v by angle
mat4  rotationMatrix(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat4(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y, 0.0,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x, 0.0,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z, 0.0,
		0.0, 0.0, 0.0, 1.0);
}

mat4 translate(vec3 v) {
	return mat4(1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		v.x,v.y,v.z,1.0);
}

mat4 scale4(float s) {
	return mat4(s,0.0,0.0,0.0,
		0.0,s,0.0,0.0,
		0.0,0.0,s,0.0,
		0.0,0.0,0.0,1.0);
}



#group default
#group Mandelbox

/*
The distance estimator below was originalled devised by Buddhi.
This optimized version was created by Rrrola (Jan Kadlec), http://rrrola.wz.cz/

See this thread for more info: http://www.fractalforums.com/3d-fractal-generation/a-mandelbox-distance-estimate-formula/15/
*/

// Number of fractal iterations.
uniform int Iterations;  slider[0,17,300]
uniform int ColorIterations;  slider[0,3,300]

//uniform float MinRad2;  slider[0,0.25,2.0]

// Scale parameter. A perfect Menger is 3.0
uniform float Scale;  slider[0,1.5,3.0]
//uniform float bailout;  slider[0,2.0,50]
uniform vec3 Fold; slider[(0,0,0),(0,0,0),(1,1,1)]
uniform vec3 Julia; slider[(-2,-2,-2),(-0.5,-0.5,-0.5),(1,1,1)]
vec4 scale = vec4(Scale, Scale, Scale, abs(Scale));

// precomputed constants

uniform vec3 RotVector; slider[(-1,-1,-1),(1,1,1),(1,1,1)]


// Scale parameter. A perfect Menger is 3.0
uniform float RotAngle; slider[-180,0,180]

mat3 rot;


//float absScalem1 = abs(Scale - 1.0);
//float AbsScaleRaisedTo1mIters = pow(abs(Scale), float(1-Iterations));
float expsmoothing = 0.0;
float l = 0.0;



// Compute the distance from `pos` to the Mandelbox.
float DE(vec3 pos) {
	 rot = rotationMatrix3(normalize(RotVector), RotAngle);
	vec3 p = pos, p0 = Julia;  // p.w is the distance estimate
	
	int i=0;
	for (i=0; i<Iterations; i++) {
		p*=rot;
		p.xy=abs(p.xy+Fold.xy)-Fold.xy;
		p=p*Scale+p0;
		l=length(p);
		if (i<ColorIterations) orbitTrap = min(orbitTrap, abs(vec4(p.xyz,expsmoothing)));
	}
	return (l)*pow(Scale, -float(i));
}



/*
FOV = 1
Eye = 0.0104856,2.35989e-05,-1.99996
Target = 3.75959,3.86772,-10.4291
Up = 0.753402,0.403223,0.51942
AntiAlias = 1
AntiAliasBlur = 1
Detail = -2.3
DetailNormal = -2.8
BackStepNormal = 1
ClarityPower = 1
MaxDist = 600
FudgeFactor = 1
MaxRaySteps = 56
MaxRayStepsDiv = 1.8
BandingSmooth = 0
BoundingSphere = 2
AO = 0.7
AOColor = 0,0,0
SpotLight = 0.4
Specular = 4
SpecularExp = 16
SpotLightColor = 1,1,1
SpotLightDir = 0.1,0.1
CamLight = 1
CamLightColor = 1,1,1
Glow = 0.2
GlowColor = 1,1,1
BackgroundColor = 0.6,0.6,0.45
GradientBackground = 0.3
BaseColor = 1,1,1
OrbitStrength = 0.8
XStrength = 0.7
X = 0.5,0.6,0.6
YStrength = 0.4
Y = 1,0.6,0
ZStrength = 0.5
Z = 0.8,0.78,1
RStrength = 0.12
R = 0.4,0.7,1
Iterations = 17
Scale = 3
RotVector = 1,1,1
RotAngle = 0
*/



#group default



#group default

#preset default
FOV = 0.4
Eye = 2.24024,-0.914052,1.26689
Target = -2.33559,2.99323,-2.33041
Up = 0.740733,0.611624,-0.277898
AntiAlias = 2
Detail = -2.72566
DetailAO = -1.92857
FudgeFactor = 0.83133
MaxRaySteps = 440
BoundingSphere = 7.5904
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,0.71429
Specular = 5.1899
SpecularExp = 7.273
SpotLight = 1,1,1,0.54902
SpotLightDir = -0.4375,-0.09374
CamLight = 1,1,1,1.34616
CamLightMin = 0
Glow = 1,1,1,0.09589
GlowMax = 52
Fog = 0.57408
HardShadow = 0.73846 NotLocked
ShadowSoft = 7.0968
Reflection = 0.11538 NotLocked
BaseColor = 1,1,1
OrbitStrength = 0.85714
X = 0.588235,0.6,0.380392,-0.3398
Y = 1,0.6,0,0.06796
Z = 1,0.231373,0.207843,0.20388
R = 0.470588,0.509804,0.854902,-0.05882
BackgroundColor = 0.117647,0.14902,0.227451
GradientBackground = 0.5435
CycleColors = false
Cycles = 4.04901
EnableFloor = true
FloorNormal = -0.68292,-1,0.12196
FloorHeight = -0.0563
FloorColor = 1,1,1
Iterations = 20
ColorIterations = 14
Scale = 1.44915
Fold = 0.13008,0,0
Julia = -0.04238,-0.80507,-0.16949
RotVector = 0.2174,1,0.02174
RotAngle = 93.2544
#endpreset

