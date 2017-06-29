

#version 150

out vec4 oColor;

uniform sampler2D uTexMap;
uniform sampler2D uTexPlanes;

uniform float uTime;

uniform vec4 uMapColor;

in vec2	TexCoord;
void main( void )
{

	vec4 tMap           = texture( uTexMap, TexCoord.st 	+ vec2(uTime, 0.0) );
	vec4 tPlanes 		= texture( uTexPlanes, TexCoord.st  + vec2(uTime, 0.0));

	vec4 compose =  mix(tMap * uMapColor, tPlanes, tPlanes.a);

	oColor =   vec4(compose.rgb, 1.0);
}
