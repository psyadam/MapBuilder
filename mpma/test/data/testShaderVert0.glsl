void main()
{
    gl_TexCoord[0].xy=gl_Vertex.xy/2+0.5;
    gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;
}
