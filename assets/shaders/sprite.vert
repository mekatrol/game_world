#version 330 core

// Static quad vertex in [0..1] range
layout(location = 0) in vec2 aPos;

// Per-instance attributes
layout(location = 1) in vec2 i_pos;   // pixels
layout(location = 2) in vec2 i_size;  // pixels
layout(location = 3) in vec4 i_uv;    // (u0, v0, u1, v1)

uniform mat4 u_proj;

out vec2 v_uv;

void main() {
    // Interpolate UV based on quad vertex position.
    // aPos is 0..1, so this maps corners correctly.
    v_uv = mix(i_uv.xy, i_uv.zw, aPos);

    // Scale + translate the unit quad into world space
    vec2 world = i_pos + aPos * i_size;

    gl_Position = u_proj * vec4(world, 0.0, 1.0);
}
