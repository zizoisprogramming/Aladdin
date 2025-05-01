#version 330 core
#include "light_common.glsl"

uniform sampler2D albedo_map;
uniform sampler2D specular_map;
uniform sampler2D roughness_map;
uniform sampler2D ao_map;
uniform sampler2D emission_map;

#define TYPE_DIRECTIONAL    0
#define TYPE_POINT          1
#define TYPE_SPOT           2
#define MAX_LIGHT_COUNT     16

uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;
uniform vec3 ambient_light;

in Varyings {
    vec4 color;
    vec2 tex_coord;
    // We will need the vertex position in the world space,
    vec3 world;
    // the view vector (vertex to eye vector in the world space),
    vec3 view;
    // and the surface normal in the world space.
    vec3 normal;
} fsin;


out vec4 frag_color;

void main() {
    vec3 albedo = texture(albedo_map, fsin_tex_coord).rgb;
    float roughness = texture(roughness_map, fsin_tex_coord).r;
    vec3 emission = texture(emission_map, fsin_tex_coord).rgb;
    float ao = texture(ao_map, fsin_tex_coord).r;
    vec3 specular_map_value = vec3(texture(specular_map, fsin_tex_coord).r);

    vec3 normal = normalize(fsin_normal);
    vec3 view = normalize(fsin_view);

    int count = min(light_count, MAX_LIGHT_COUNT);
    vec3 accumulated_light = emission + albedo * ambient_light * ao;

    for (int index = 0; index < count; index++) {
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1.0f;

        if (light.type == TYPE_DIRECTIONAL) {
            light_direction = normalize(-light.direction);
        } else {
            light_direction = fsin_world - light.position;
            float distance = length(light_direction);
            light_direction = normalize(light_direction);

            attenuation *= 1.0f / (light.attenuation_constant +
                light.attenuation_linear * distance +
                light.attenuation_quadratic * distance * distance);

            if (light.type == TYPE_SPOT) {
                float angle = acos(dot(normalize(light.direction), -light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        float diff = calculate_lambert(normal, light_direction);
        float shininess = 2.0 / pow(clamp(roughness,0.001,0.999), 4.0) - 2.0;
        float spec = calculate_phong(normal, light_direction, view, shininess);

        vec3 diffuse_component = albedo * light.diffuse * diff;
        vec3 specular_component = specular_map_value * light.specular * spec;
        vec3 ambient_component = albedo * light.ambient * ao;

        accumulated_light += (diffuse_component + specular_component + ambient_component) * attenuation;
    }

    frag_color = vec4(accumulated_light, 1.0f);
}
