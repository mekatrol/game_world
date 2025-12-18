#version 330 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_texture;

void main() {
    vec4 c = texture(u_texture, v_uv);

    // Treat near-black as transparent
    float eps = 5.0 / 255.0;
    
    if(all(lessThan(c.rgb, vec3(eps)))) {
        c.a = 0.0;
    }

    frag_color = c;
}
