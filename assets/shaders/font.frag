#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;     // font atlas (font.png)
uniform vec4 uFontColor;        // e.g. (1,1,1,1)
uniform float uPxRange;         // MUST match generator's rangePx (yours is 6) :contentReference[oaicite:0]{index=0}

float median3(float a, float b, float c) {
    return max(min(a, b), min(max(a, b), c));
}

void main() {
    // Your generator writes MTSDF into RGBA (generateMTSDF) :contentReference[oaicite:1]{index=1}
    vec4 s = texture(uTexture, vUV);

    // Use the MSDF part (RGB). (A can be used for true-distance in MTSDF, but RGB median is the common path.)
    float sd = median3(s.r, s.g, s.b) - 0.5;

    // Scale smoothing based on derivatives (works across font sizes without a per-size uniform)
    float w = fwidth(sd);
    float alpha = smoothstep(-w, w, sd);

    FragColor = vec4(uFontColor.rgb, uFontColor.a * alpha);
}
