#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{

   
//const float gamma = 2.2;
//vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
//
// vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
// // gamma correction 
// mapped = pow(mapped, vec3(1.0 / gamma));
//
    vec2 flippedCoords = TexCoords;
    flippedCoords.y = 1.0 - flippedCoords.y;
    FragColor = texture(screenTexture, flippedCoords);
}