#version 150

out vec4 oColor;

uniform sampler2D uTexMap; 
uniform sampler2D uTexPlanes;
uniform sampler2D uTexTerminator;

uniform float uTime;

in vec2	TexCoord;
void main( void )
{	

	vec4 tMap           = texture( uTexMap, TexCoord.st 	+ vec2(uTime, 0.0) ) * 0.4;
	vec4 tPlanes 		= texture( uTexPlanes, TexCoord.st  + vec2(uTime, 0.0));
	vec4 tTerminator    = texture( uTexTerminator, TexCoord.st);

	vec4 compose =  mix(tMap, tPlanes, tPlanes.a) * vec4(0.75, 0.3, 0.3, 1.0);

	vec3 inverted = abs(tTerminator.rgb - compose.rgb);
	oColor =  vec4(inverted, 1.0);
}