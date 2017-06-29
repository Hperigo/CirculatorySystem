#version 150

out vec4 oColor;

uniform sampler2D uTexTerminator;

uniform vec2		sample_offset;
uniform float		attenuation;

in vec2	TexCoord;
void main( void )
{
	vec3 sum = vec3( 0.0, 0.0, 0.0 );
	sum += texture( uTexTerminator, TexCoord + -10.0 * sample_offset ).rgb * 0.009167927656011385;
	sum += texture( uTexTerminator, TexCoord +  -9.0 * sample_offset ).rgb * 0.014053461291849008;
	sum += texture( uTexTerminator, TexCoord +  -8.0 * sample_offset ).rgb * 0.020595286319257878;
	sum += texture( uTexTerminator, TexCoord +  -7.0 * sample_offset ).rgb * 0.028855245532226279;
	sum += texture( uTexTerminator, TexCoord +  -6.0 * sample_offset ).rgb * 0.038650411513543079;
	sum += texture( uTexTerminator, TexCoord +  -5.0 * sample_offset ).rgb * 0.049494378859311142;
	sum += texture( uTexTerminator, TexCoord +  -4.0 * sample_offset ).rgb * 0.060594058578763078;
	sum += texture( uTexTerminator, TexCoord +  -3.0 * sample_offset ).rgb * 0.070921288047096992;
	sum += texture( uTexTerminator, TexCoord +  -2.0 * sample_offset ).rgb * 0.079358891804948081;
	sum += texture( uTexTerminator, TexCoord +  -1.0 * sample_offset ).rgb * 0.084895951965930902;
	sum += texture( uTexTerminator, TexCoord +   0.0 * sample_offset ).rgb * 0.086826196862124602;
	sum += texture( uTexTerminator, TexCoord +  +1.0 * sample_offset ).rgb * 0.084895951965930902;
	sum += texture( uTexTerminator, TexCoord +  +2.0 * sample_offset ).rgb * 0.079358891804948081;
	sum += texture( uTexTerminator, TexCoord +  +3.0 * sample_offset ).rgb * 0.070921288047096992;
	sum += texture( uTexTerminator, TexCoord +  +4.0 * sample_offset ).rgb * 0.060594058578763078;
	sum += texture( uTexTerminator, TexCoord +  +5.0 * sample_offset ).rgb * 0.049494378859311142;
	sum += texture( uTexTerminator, TexCoord +  +6.0 * sample_offset ).rgb * 0.038650411513543079;
	sum += texture( uTexTerminator, TexCoord +  +7.0 * sample_offset ).rgb * 0.028855245532226279;
	sum += texture( uTexTerminator, TexCoord +  +8.0 * sample_offset ).rgb * 0.020595286319257878;
	sum += texture( uTexTerminator, TexCoord +  +9.0 * sample_offset ).rgb * 0.014053461291849008;
	sum += texture( uTexTerminator, TexCoord + +10.0 * sample_offset ).rgb * 0.009167927656011385;

	oColor.rgb = vec3(1.0,0.0, 0.0) * sum;
	oColor.a = 1.0;
}
