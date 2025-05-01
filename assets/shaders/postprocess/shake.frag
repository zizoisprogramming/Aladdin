#version 330

uniform sampler2D tex;
uniform float time;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    // Shake effect by modifying the texture coordinates slightly based on time
    float strength = 0.01;
    vec2 offset = vec2(
        sin(time * 10.0) * strength,
        cos(time * 15.0) * strength
    );

    vec2 shaken_coord = tex_coord + offset;
    
    // Sample the texture at the shaken coordinates
    frag_color = texture(tex, shaken_coord);
}
