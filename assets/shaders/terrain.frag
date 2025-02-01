#version 330 core

out vec4 FragColor;

in float Height;
in vec2 texCoord;
uniform vec3 color;

uniform sampler2D outTexture1;

void main()
{
    vec3 text = texture(outTexture1, texCoord * 10).rgb;
    float h = (Height + 16)/32.0f;	// shift and scale the height into a grayscale value
    vec3 colored = text * h;
    FragColor = vec4(colored, 1.0);
}