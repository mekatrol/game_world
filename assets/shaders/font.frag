#version 330 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_texture; // font atlas (font.png)
uniform vec4 u_color;        // r, g, b, a

float median3(float a, float b, float c) {
    return max(min(a, b), min(max(a, b), c));
}

void main() {
    vec4 s = texture(u_texture, v_uv);

    float sd = median3(s.r, s.g, s.b) - 0.5;

    // Scale smoothing based on derivatives (works across font sizes without a per-size uniform)
    float w = fwidth(sd);
    float alpha = smoothstep(-w, w, sd);

    frag_color = vec4(u_color.rgb, u_color.a * alpha);
}
