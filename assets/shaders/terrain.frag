#version 330 core

out vec4 FragColor;

in float Height;
in vec2 texCoord;


uniform sampler2D ourTexture1; //grass
uniform sampler2D ourTexture2; //rock
uniform float yScale;
uniform float yShift;



void main()
{
    float heightMax = yScale-yShift;
    float heightMin = -yScale-yShift;
    
    // vec3 text = mix(texture(ourTexture2, texCoord * 10).rgb, texture(ourTexture1, texCoord * 10).rgb, h);

    vec3 text = texture(Height> (0.3-yShift) ?ourTexture1: ourTexture2, texCoord * 10).rgb ; // 4.0f is the y_shift
    float delta = heightMax - heightMin;
    if (delta < 0.0001) {
        delta = 0.0001; // prevent division by zero or very small numbers
    }
    float h = (Height - heightMin) / delta; // shift and scale the height into a grayscale value  (basically normalising)
    vec3 colored = text * h;
    FragColor = vec4(colored, 1.0);
}