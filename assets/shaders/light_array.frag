#version 330 core
// This contains all the material properties in a single struct.
    struct Light {
    int type;                  // Light type (directional, point, spot)
    vec3 color;                // Color of the light (diffuse, ambient, specular)
    vec3 position;             // Position of the light (used for point and spot lights)
    vec3 direction;            // Direction of the light (used for directional and spot lights)
    vec3 attenuation;          // Attenuation factors (constant, linear, quadratic)
    vec2 spot_angle;           // Inner and outer angle for spotlights
};
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
    vec3 world;      // Vertex position in world space
    vec3 view;       // View vector (vertex to eye vector in world space)
    vec3 normal;     // Surface normal in world space
} fsin;

out vec4 frag_color;

    // This will be used to compute the diffuse factor.
    float calculate_lambert(vec3 normal, vec3 light_direction){
        return max(0.0f, dot(normal, -light_direction));
    }

    // This will be used to compute the phong specular.
    float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
        vec3 reflected = reflect(light_direction, normal);
        return pow(max(0.0f, dot(view, reflected)), shininess);
    }

    

void main() {
    // Fetch textures
    vec3 albedo = texture(albedo_map, fsin.tex_coord).rgb;
    float roughness = texture(roughness_map, fsin.tex_coord).r;
    vec3 emission = texture(emission_map, fsin.tex_coord).rgb;
    float ao = texture(ao_map, fsin.tex_coord).r;
    vec3 specular_map_value = vec3(texture(specular_map, fsin.tex_coord).r);

    // Normalize normal and view vectors
    vec3 normal = normalize(fsin.normal);
    vec3 view = normalize(fsin.view);

    // Initialize the accumulated light (starting with emission and ambient light)
    int count = min(light_count, MAX_LIGHT_COUNT);
    vec3 accumulated_light = emission + albedo * ambient_light * ao;

    for (int index = 0; index < count; index++) {
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1.0f;

        // Handle directional light
        if (light.type == TYPE_DIRECTIONAL) {
            light_direction = normalize(-light.direction);
        } else {
            // Handle point and spot lights
            light_direction = fsin.world - light.position;
            float distance = length(light_direction);
            light_direction = normalize(light_direction);

            // Calculate attenuation based on distance
            attenuation *= 1.0f / (light.attenuation.x + 
                light.attenuation.y * distance + 
                light.attenuation.z * distance * distance);

            // Handle spotlights with angle-based attenuation
            if (light.type == TYPE_SPOT) {
                float angle = acos(dot(normalize(light.direction), -light_direction));
                attenuation *= smoothstep(light.spot_angle.y, light.spot_angle.x, angle);
            }
        }

        // Lambertian diffuse and Phong specular calculations
        float diff = calculate_lambert(normal, light_direction);
        float shininess = 2.0 / pow(clamp(roughness, 0.001, 0.999), 4.0) - 2.0;
        float spec = calculate_phong(normal, light_direction, view, shininess);

        // Compute light components (diffuse, specular, ambient)
        vec3 diffuse_component = albedo * light.color * diff;
        vec3 specular_component = specular_map_value * light.color * spec;
        vec3 ambient_component = albedo * light.color * ao;

        // Accumulate light contributions
        accumulated_light += (diffuse_component + specular_component + ambient_component) * attenuation;
    }

    // Output final fragment color
    frag_color = vec4(accumulated_light, 1.0f);
}
