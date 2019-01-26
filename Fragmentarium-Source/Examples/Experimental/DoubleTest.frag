#version 400 compatibility

#define USE_DOUBLE

#include "MathUtils.frag"
#include "Complex.frag"
#include "Progressive2D-4.frag"

#info BurningShirp inspired by the good folks at FractalForums.com 2013-2017
#group BurningShirp

uniform int  RetryMax; slider[0,0,64]
uniform double RetryEntry; slider[0,0,64]
uniform double RetryBias; slider[0,0,64]

// Number of iterations
uniform int  Iterations; slider[10,200,10000]
uniform int  PreIter; slider[0,0,1000]

uniform double R; slider[0,0,1]
uniform double G; slider[0,0.4,1]
uniform double B; slider[0,0.7,1]
uniform double ColDiv; slider[1,256,384]

uniform double Power; slider[1,2,10]
uniform double Bailout; slider[0,6,384]

// 0=BurningShip 1=Perpendicular BurningShip 2=Perpendicular Mandelbrot
uniform int  Formula; slider[0,0,2]

uniform bool Julia; checkbox[false]
uniform dvec2 JuliaXY; slider[(-2,-2),(-0.6,1.3),(2,2)]

// Meta
uniform bool Meta; checkbox[false]
// Sin
uniform bool Sin; checkbox[false]

double dist = 0.;
dvec2 zmin = dvec2(1000000.0);

// Pickover's Stalks
uniform bool StalksInside; checkbox[false]
uniform bool StalksOutside; checkbox[false]

uniform bool Invert; checkbox[false]
// coordinate to invert to infinity
uniform dvec2 InvertC; slider[(-5,-5),(0,0),(5,5)]
// performs the active c = T(s)
dvec2 domainMap(dvec2 c)
{
	double s = dot(c,c);
	return c/s + InvertC;
}

dvec2 Plot(dvec2 a) {
	
	dvec2 b;
	
	b.x = sqrt ( a.x * a.x + a.y * a.y );
	b.y = atan ( (a.y / a.x), 1.0 );
	
	if ( a.x < 0.) b.y += M_PI;
	
	b.x = pow( float(b.x) , float(Power) );
	b.y *= Power;
	
	a.x = cos( b.y ) * b.x;
	a.y = sin( b.y ) * b.x;

	return a;
}

dvec3 IQColor(double i, dvec2 zmin, dvec2 z) {
	// The color scheme here is based on one from Inigo Quilez's Shader Toy:
	// http://www.iquilezles.org/www/articles/mset_smooth/mset_smooth.htm
	double co;
	double p = Power*Power;
	
	if(Meta && Sin) co = i;
    //co = i + 1. - log(.5+.5*log2(length(z))/(.5+.5*log2(Power)))/log2(Bailout);
	//co = i - log(log2( dist )/log2( Bailout))/log2(Bailout );
	else
	if(!Meta && !Sin)
	  co = i - log(log( length(z) ))/log( Power );  // equivalent optimized smooth interation count
	else
	if(Meta && !Sin)
	  co = i - log(.5*log( dist ) / log( p ))/log( p );
	else
	if(!Meta && Sin)
	  co =  i - (log2(log( dist ))- log2(log( p ))) / log( Bailout );
	
	co = M_2PI*sqrt( co/ColDiv );
	
	return .5+.5*dvec3( cos( co+R ), cos( co+G ), cos( co+B ));
}

dvec3 StalkColor( double i,dvec2 zmin) {
	dvec3 c;
	c.x = max(zmin.x, zmin.y)*R;
	c.y = min(zmin.x, zmin.y)*G;
	c.z = length(zmin)*B;
	c = sqrt(c*i);
	double p = sqrt(min(zmin.x, zmin.y)*12.0);
	return sqrt(c * dvec3(p));
}

dvec3 color(dvec2 c) {
	int retry = 0;
	if(Invert) c = domainMap(-c);
	
	dvec2 z = Julia ?  c : dvec2(0.0,0.0);
	
	c = (Julia ? JuliaXY : c);
	
	int i;
	
	for(i=0;i<PreIter;i++) {
/*		if(Meta&&Sin) z = cSin(Plot( Plot(z)+c) ) + Plot(c) + z;
		else if(Meta&&!Sin) z = Plot( Plot(z)+c ) + Plot(c) + z;
		else if(!Meta&&Sin) z = cSin( Plot(z) )+c;
		else */
       z = Plot(z) + c;
	}

	for (i = 0; i < Iterations; i++) {

		
		if(Formula == 0) z=abs(z);// Burning Ship
		else if(Formula == 1) z.y=abs(z.y); // Perpendicular Burning Ship
		else if(Formula == 2) z.x=abs(z.x); // Perpendicular Mandelbrot
		
		z.y = -z.y; // invert y
		
		if(Meta&&Sin) z = cSin( dvec2(Plot( Plot(z)+c)) ) + Plot(c) + z;
		else if(Meta&&!Sin) z = Plot( Plot(z)+c ) + Plot(c) + z;
		else if(!Meta&&Sin) z = cSin( dvec2(Plot(z)) )+c;
		else z = Plot(z) + c;
    // Chris Thomasson
    // http://www.fractalforums.com/index.php?action=gallery;sa=view;id=20565
    if (z.x * z.x + z.y * z.y > RetryEntry)
    {
        if (retry < RetryMax)
        {
            z = z * (1./RetryBias);
            --i;
            ++retry;
            continue;
        }
    }

		dist = dot(z,z);
		zmin = min(zmin, abs(z));
		if (dist> Bailout) break;

	}
	
	dvec3 rColor;
	
	if (i < Iterations) {
		rColor = StalksOutside?StalkColor((double(i+PreIter)+.5)*.01,zmin): IQColor(double(i+PreIter)+M_PI,zmin,z);
	}  else {
		rColor = StalksInside?StalkColor((double(i+PreIter)+.5)*.01,zmin): dvec3(.0);
	}
	
	return rColor;
}

#preset Default
Gamma = 2
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.241051197052,0.38701269030571
Zoom = 0.64841693546845
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
Iterations = 1000
PreIter = 0
R = 0.25
G = 0.5
B = 0.75
ColDiv = 10
Power = 2
Bailout = 45
Formula = 0
Julia = false
JuliaXY = -0.22222219407558,0
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = false
InvertC = 0,0
RetryEntry = 0
RetryBias = 0
RetryMax = 0
#endpreset


#preset CT-retry
Gamma = 2
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.49432203173637,0.01609551906586
Zoom = 0.46296045184135
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
Iterations = 1000
PreIter = 2
R = 0.25
G = 0.5
B = 0.75
ColDiv = 10
Power = 2
Bailout = 45
Formula = 0
Julia = false
JuliaXY = -0.22222219407558,0
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = false
InvertC = 0,0
RetryEntry = 16
RetryBias = 5
RetryMax = 4
#endpreset

#preset CT-Scan
Gamma = 2
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.00272041559219,0.02782937884331
Zoom = 0.88941305875778
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
RetryMax = 4
RetryEntry = 15
RetryBias = 8
Iterations = 500
PreIter = 2
R = 0.25
G = 0.5
B = 0.75
ColDiv = 4
Power = 2
Bailout = 384
Formula = 0
Julia = true
JuliaXY = 0.25641027092934,0
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = false
InvertC = 0,0
#endpreset

#preset RightHemisphere
Gamma = 1
Brightness = 1
Contrast = 1
Saturation = 1
Center = 0.30667564272881,0.91197913885117
Zoom = 5.4723681832482
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
RetryMax = 5
RetryEntry = 14
RetryBias = 8
Iterations = 500
PreIter = 2
R = 0.25
G = 0.5
B = 0.75
ColDiv = 3
Power = 1.9919998487458
Bailout = 384
Formula = 0
Julia = true
JuliaXY = 0.25641027092934,0
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = false
InvertC = 0,0
#endpreset

#preset tree
Gamma = 1
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.01211503148079,0.97422331571579
Zoom = 1.7889271629482
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
RetryMax = 6
Iterations = 500
PreIter = 2
R = 0.25
G = 0.5
B = 0.75
ColDiv = 3
Power = 1.9939997876063
Bailout = 384
Formula = 0
Julia = true
JuliaXY = 0.73666665330529,0.04128205403686
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = false
InvertC = 0,0
RetryEntry = 32
RetryBias = 10
#endpreset

#preset Details
Gamma = 4
Brightness = 1
Contrast = 1
Saturation = 1
Center = -0.0845739915967,-0.44618257880211
Zoom = 326.47360567683
Exposure = 1
ToneMapping = 1
AARange = 1
AAExp = 2
GaussianAA = true
RetryMax = 11
RetryEntry = 57
RetryBias = 20
Iterations = 40
PreIter = 2
R = 0.25
G = 0.5
B = 0.75
ColDiv = 12
Power = 1.9939997876063
Bailout = 64
Formula = 0
Julia = true
JuliaXY = 0.63466686010361,0.04728204384446
Meta = false
Sin = false
StalksInside = false
StalksOutside = false
Invert = true
InvertC = 0,0
#endpreset
