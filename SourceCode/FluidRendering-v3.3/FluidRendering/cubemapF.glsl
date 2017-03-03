#version 430

in vec3 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform samplerCube uSkyBoxSampler;

void main()
{    
    FragColor = texture(uSkyBoxSampler, TexCoords);
}