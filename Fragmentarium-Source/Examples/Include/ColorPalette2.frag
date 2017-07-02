#define providesColor
#group ColorPalette
uniform vec3 Color1; color[1.0,0.0,0.0];
uniform vec3 Color2; color[0.0,1.0,0.0];
uniform vec3 Color3; color[0.0,0.0,1.0];
uniform float ColorDensity; slider[0.0,1.0,1.0];
uniform float ColorOffset; slider[0.0,0.0,10.0];


mat3 rotMat3(vec3 v, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

float Coloring(vec3 p);


vec3 color(vec3 p, vec3 n)
{
	float colorindex=Coloring(p);
	vec3 colrot=vec3(1,0,0)*rotMat3(normalize(vec3(1,1,1)),colorindex*ColorDensity+ColorOffset);
	return Color1*colrot.x+Color2*colrot.y+Color3*colrot.z;
}

