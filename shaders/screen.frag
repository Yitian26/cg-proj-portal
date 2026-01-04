#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 ClipPos;

uniform sampler2D reflectionTexture;

void main()
{
    // Calculate Normalized Device Coordinates (NDC)
    vec2 ndc = (ClipPos.xy / ClipPos.w) * 0.5 + 0.5;

    vec4 reflectionColor = texture(reflectionTexture, ndc);
    
    FragColor = reflectionColor;
}
