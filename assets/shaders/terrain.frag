#version 330 core

out vec4 FragColor;

in float Height;
uniform vec3 color;

void main()
{
    float h = (Height + 16)/32.0f;	// shift and scale the height into a grayscale value
    vec3 colored = color * h;
    FragColor = vec4(colored, 1.0);
}