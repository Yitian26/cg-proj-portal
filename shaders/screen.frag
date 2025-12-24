#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 ClipPos;

uniform sampler2D reflectionTexture;

void main()
{
    // Calculate Normalized Device Coordinates (NDC)
    vec2 ndc = (ClipPos.xy / ClipPos.w) * 0.5 + 0.5;
    
    // Sample the reflection texture
    // For Portals, we do NOT need to flip X because the 180 degree rotation
    // keeps the coordinate system consistent (Right-Handed).
    // However, we are using the same shader as the mirror for now.
    // If this shader is used for portals, we should use standard NDC.
    // Let's assume this shader is now dedicated to Portals/Mirrors and we adjust based on usage.
    // Since we removed the Mirror class usage, let's revert to standard sampling for Portals.
    vec4 reflectionColor = texture(reflectionTexture, ndc);
    
    FragColor = reflectionColor;
}
