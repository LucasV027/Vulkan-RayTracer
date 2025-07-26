#version 450

layout (location = 0) in vec2 inColor;

layout (location = 0) out vec4 outColor;

void main() {
    vec2 scaled = (inColor + vec2(1.)) / 2.; // [-1, 1] -> [0, 1]
    outColor = vec4(scaled.x, scaled.y, scaled.x, 1.0);
}