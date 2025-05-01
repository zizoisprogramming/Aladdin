#ifndef OUR_LIGHT_COMMON_GLSL_INCLUDED
#define OUR_LIGHT_COMMON_GLSL_INCLUDED

    // These are some common functions and data structures for all types of lights so we wrote them in a single file to be included in the other files.

    // This will be used to compute the diffuse factor.
    float calculate_lambert(vec3 normal, vec3 light_direction){
        return max(0.0f, dot(normal, -light_direction));
    }

    // This will be used to compute the phong specular.
    float calculate_phong(vec3 normal, vec3 light_direction, vec3 view, float shininess){
        vec3 reflected = reflect(light_direction, normal);
        return pow(max(0.0f, dot(view, reflected)), shininess);
    }

    // This contains all the material properties in a single struct.
    struct Light {
    int type;                  // Light type (directional, point, spot)
    vec3 color;                // Color of the light (diffuse, ambient, specular)
    vec3 position;             // Position of the light (used for point and spot lights)
    vec3 direction;            // Direction of the light (used for directional and spot lights)
    vec3 attenuation;          // Attenuation factors (constant, linear, quadratic)
    vec2 spot_angle;           // Inner and outer angle for spotlights
};



#endif