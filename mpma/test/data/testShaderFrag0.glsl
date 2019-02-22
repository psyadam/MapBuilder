uniform float time;
uniform sampler2D testSampler;

void main()
{
    vec4 texColor=texture2D(testSampler, gl_TexCoord[0].xy);
    vec4 texMod=1+vec4(sin(gl_TexCoord[0].x*200+time*7), cos(gl_TexCoord[0].y*200+time*2), sin(gl_TexCoord[0].x*200+time*30*gl_TexCoord[0].y), 1);
    gl_FragColor=texColor*texMod;
}
