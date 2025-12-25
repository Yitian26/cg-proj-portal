#version 330 core
layout(location = 0) in vec2 aPos; // NDC coordinate

uniform vec2 uScale; // x/y scale to compensate for aspect ratio (NDC x scaled by 1/aspect)

void main() {
    vec2 p = aPos * uScale;
    gl_Position = vec4(p.xy, 0.0, 1.0);
}
