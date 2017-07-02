#version 400 compatibility

#include "Complex.frag"
#include "Progressive2D-4.frag"
#info BurningShirp inspired by the good folks at FractalForums.com 2013-2017
#group BurningShirp

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
	b.y = atan ( float(a.y / a.x) );
	
	if ( a.x < 0.) b.y += 3.141592;
	
	b.x = pow( float(b.x) , float(Power) );
	b.y *= Power;
	
	a.x = cos( float(b.y) ) * b.x;
	a.y = sin( float(b.y) ) * b.x;

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
	  co = i - log(log( float(length(z)) ))/log( float(Power) );  // equivalent optimized smooth interation count
	else
	if(Meta && !Sin)
	  co = i - log(.5*log( float(dist) ) / log( float(p) ))/log( float(p) );
	else
	if(!Meta && Sin)
	  co =  i - (log2(log( float(dist) ))- log2(log( float(p) ))) / log( float(Bailout) );
	
	co = 6.2831*sqrt( co/ColDiv );
	
	return .5+.5*dvec3( cos( float(co+R) ), cos( float(co+G) ), cos( float(co+B) ));
}

dvec3 StalkColor( double i,dvec2 zmin) {
	dvec3 c;
	c.x = max(zmin.x, zmin.y)*R;
	c.y = min(zmin.x, zmin.y)*G;
	c.z = length(zmin)*B;
	c = sqrt(c*i);
	double p = sqrt(min(zmin.x, zmin.y)*12.);
	return sqrt(c * dvec3(p));
}

dvec3 color(dvec2 c) {
	
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
		
		if(Meta&&Sin) z = cSin( vec2(Plot( Plot(z)+c)) ) + Plot(c) + z;
		else if(Meta&&!Sin) z = Plot( Plot(z)+c ) + Plot(c) + z;
		else if(!Meta&&Sin) z = cSin( vec2(Plot(z)) )+c;
		else z = Plot(z) + c;

		dist = dot(z,z);
		zmin = min(zmin, abs(z));
		if (dist> Bailout) break;
	}
	
	dvec3 rColor;
	
	if (i < Iterations) {
		rColor = StalksOutside?StalkColor((double(i+PreIter)+.5)*.01,zmin): IQColor(double(i+PreIter)+3.14159,zmin,z);
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
#endpreset

