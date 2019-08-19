#version 400 compatibility

#donotrun
// This is a simple shader for rendering images
// from an accumulated buffer.

#vertex

out vec2 coord;

void main(void)
{
	gl_Position =  gl_Vertex;
	coord = (gl_ProjectionMatrix*gl_Vertex).xy;
}

#endvertex

in vec2 coord;
out vec4 FragColor;

#group Post

uniform double Gamma;
uniform double Exposure; slider[0.0,1.0,30.0]
uniform double Brightness; slider[0.0,1.0,5.0]
uniform double Contrast; slider[0.0,1.0,5.0]
uniform double Saturation; slider[0.0,1.0,5.0]
// 1: Linear, 2: Exponential, 3: Filmic, 4: Reinhart
uniform int ToneMapping; slider[1,1,4]

/*
** Based on: http://mouaif.wordpress.com/2009/01/22/photoshop-gamma-correction-shader/
**
** Contrast, saturation, brightness
** Code of this function is from TGM's shader pack
** http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=21057
*/
// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
dvec3 ContrastSaturationBrightness(dvec3  color, double brt, double sat, double con)
{
	const dvec3 LumCoeff = dvec3(0.2125, 0.7154, 0.0721);
	dvec3 AvgLumin = dvec3(0.5);
	dvec3 brtColor = color * brt;
	double intensityf = dot(brtColor, LumCoeff);
	dvec3 intensity = dvec3(intensityf, intensityf, intensityf);
	dvec3 satColor = mix(intensity, brtColor, sat);
	dvec3 conColor = mix(AvgLumin, satColor, con);
	// https://fractalforums.org/fragmentarium/17/i-cant-get-the-newest-version-to-work/2629/msg13086#msg13086
	return clamp(conColor,0.0,1.0);
}

double sigmoid(double t) {
	double K = 1.0-1./(1.0+exp( float(-0.5*Contrast*5.) ));
	t -= 0.5;
	double  x = 1./(1.0+exp( float(-t*Contrast*5.) ))-K;
	return x/((1.0-2.0*K));
}

dvec3 sigmoid3(dvec3 t) {
	return dvec3(sigmoid(t.x),sigmoid(t.y),sigmoid(t.z));
}


uniform sampler2D frontbuffer;

void main() {
	vec2 pos = (coord+1.0) * 0.5;
	vec4 tex = texture(frontbuffer, vec2(pos) );
	dvec3 c = tex.xyz/tex.a;

	if (ToneMapping==1) {
		// Linear
		c = c*Exposure;
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==2) {
		// ExponentialExposure
		c = dvec3(1.0)-exp(float(-c*Exposure));
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==3) {
		// Filmic: http://filmicgames.com/archives/75
		c*=Exposure;
		dvec3 x = max(dvec3(0.),c-dvec3(0.004));
		c = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
		c = dvec3(pow(vec3(c), vec3(2.2))); // It already takes the Gamma into acount
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);

	} else if (ToneMapping==4) {
		// Reinhart
		c*=Exposure;
		c = c/(1.+c);
		c = ContrastSaturationBrightness(c, Brightness, Saturation, Contrast);
	}  else if (ToneMapping==5) {
		c = sigmoid3(c*Exposure+dvec3(Brightness-1.0));

	}
	c = pow(vec3(c), vec3(1.0/Gamma));

	FragColor = vec4(c,1.0);

}
