// An example of Image Based Lighting.
//
// NOTICE:
//  The Ditch-River Panorama HDR is authored by 'Blotchi' and copyrighted by http://www.hdrlabs.com/sibl/archive.html
//  It is licensed under a CC3.0 license: http://creativecommons.org/licenses/by-nc-sa/3.0/us/
#include "MathUtils.frag"
#include "QuilezLib.frag"
#include "IBL-Raytracer.frag"
#group TestScene
uniform float TextureScale; slider[0,1,40]

// Just a simple test scene...
float DE(vec3 p)
{
       p.xy=p.yx;
	float d = length(p+vec3(0.0,0.0,-0.25))-0.25;   //  sphere
	d= min(d, udRoundBox(p+vec3(0.2,1.0,-0.25),vec3(0.2),0.05)); // rounded box
	d =min(d, sdCone(p+vec3(1.,1.,-0.5),vec2(0.95,0.45))); // cone
	d = min(d, sdTorus88(p+vec3(-1.4,1.,-0.6),vec2(0.5,0.1))); // torus
	return min(p.z,d);// distance estimate
}

#preset Default
FOV = 0.58536
Eye = 0.4287087,-1.904756,0.4005014
Target = -4.20519,6.935317,-0.2159759
Up = -0.0180112,0.0601384,0.9977499
EquiRectangular = false
AutoFocus = false
FocalPlane = 1
Aperture = 0
Gamma = 2.2685
ToneMapping = 3
Exposure = 0.82653
Brightness = 1
Contrast = 1
Saturation = 1
GaussianWeight = 1
AntiAliasScale = 2
DepthToAlpha = false
Detail = -3.65484
DetailAO = -1.14289
FudgeFactor = 1
MaxRaySteps = 198
BoundingSphere = 4.3373
Dither = 0.5
NormalBackStep = 1
AO = 0,0,0,1
CamLight = 1,1,1,0
CamLightMin = 0
Glow = 1,1,1,0
GlowMax = 20
Fog = 0
Shadow = 0.70455
Background = Ditch-River_2k.hdr
Specular = Ditch-River_Env.hdr
Diffuse = Ditch-River_Env.hdr
EnvSpecular = 1
EnvDiffuse = 0
SpecularMax = 100
Sun = 1.130111,0.8213908
SunSize = 0.0045
DebugSun = true
RotateMap = 1,1
BaseColor = 1,1,1
OrbitStrength = 0
X = 0.5,0.6,0.6,0.2126
Y = 1,0.6,0,0.30708
Z = 0.8,0.78,1,0.35434
R = 0.666667,0.666667,0.498039,0.03174
BackgroundColor = 1,1,1
GradientBackground = 0.4348
CycleColors = false
Cycles = 1.1
EnableFloor = true
FloorNormal = 0,0,0.17074
FloorHeight = 0
FloorColor = 1,1,1
ShowFloor = false
TextureScale = 1
#endpreset
