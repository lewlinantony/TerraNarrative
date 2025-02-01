#version 330 core

out vec4 FragColor;

in float Height;
in vec2 texCoord;

uniform vec3 color;
uniform sampler2D ourTexture1; //grass
uniform sampler2D ourTexture2; //rock



void main()
{
    vec3 text = texture(Height> (0.3-4.0) ?ourTexture1: ourTexture2, texCoord * 10).rgb ; // 4.0f is the y_shift
    float h = (Height + 16)/32.0f;	// shift and scale the height into a grayscale value
    vec3 colored = text * h;
    FragColor = vec4(text, 1.0);
}