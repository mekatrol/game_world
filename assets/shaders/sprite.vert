#version 330 core

// Static quad vertex in [0..1] range
layout(location = 0) in vec2 aPos;

// Per-instance attributes
layout(location = 1) in vec2 iPos;   // pixels
layout(location = 2) in vec2 iSize;  // pixels
layout(location = 3) in vec4 iUV;    // (u0, v0, u1, v1)

uniform mat4 uProj;

out vec2 vUV;

void main() {
    // Interpolate UV based on quad vertex position.
    // aPos is 0..1, so this maps corners correctly.
    vUV = mix(iUV.xy, iUV.zw, aPos);

    // Scale + translate the unit quad into world space
    vec2 world = iPos + aPos * iSize;

    gl_Position = uProj * vec4(world, 0.0, 1.0);
}
