#version 330 core

out vec4 FragColor;

in float Height;
in vec2 texCoord;

uniform sampler2D ourTexture1; // grass
uniform sampler2D ourTexture2; // rock
uniform sampler2D ourTexture3; // rock
uniform float heightMin; 
uniform float heightMax;

void main()
{
    // Normalize the height value
    float delta = heightMax - heightMin;
    if (delta < 0.0001) {
        delta = 0.0001; // Prevent division by zero
    }
    float h = (Height - heightMin) / delta;

    // Texture sampling
    vec3 tex1 = texture(ourTexture1, texCoord * 10.0).rgb;
    vec3 tex2 = texture(ourTexture2, texCoord * 10.0).rgb;
    vec3 tex3 = texture(ourTexture3, texCoord * 10.0).rgb;

    // Smooth transition ranges
    float blend1 = smoothstep(0.4, 0.6, h); // Transition between grass and rock
    float blend2 = smoothstep(0.8, 0.9, h); // Transition between rock and high rock

    // Blending
    vec3 blendedTexture = mix(tex1, tex2, blend1);  // First blend
    blendedTexture = mix(blendedTexture, tex3, blend2); // Second blend

    // Apply height-based lighting
    vec3 colored = blendedTexture * h;
    
    FragColor = vec4(colored, 1.0);
}