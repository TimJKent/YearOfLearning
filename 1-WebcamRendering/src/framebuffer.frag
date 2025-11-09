#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    //Flip image to account for YUV format
    vec2 flippedCoords = TexCoords;
    flippedCoords.y = 1.0 - flippedCoords.y;

    FragColor = texture(screenTexture, flippedCoords);
}