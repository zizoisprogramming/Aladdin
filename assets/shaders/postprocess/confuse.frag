#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    // Invert the texture coordinates to create a confuse effect (image flip)
    vec2 confused_coord = vec2(1.0 - tex_coord.x, 1.0 - tex_coord.y);
    
    // Sample the texture with the inverted coordinates
    frag_color = texture(tex, confused_coord);
}
