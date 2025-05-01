#pragma once

#include "../ecs/component.hpp"
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>  
#include <glm/glm.hpp>

namespace our {

    class LightComponent : public Component {
    public:
        // Light color (RGB)
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); 

        // Type of light (Point, Directional, or Spot)
        enum class LightType {
            Point,
            Directional,
            Spot
        };
        LightType type = LightType::Point;  // default is point light
        glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f); // Direction of the light (used for Directional and Spot lights)
        glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f); 
        struct {
            float constant, linear, quadratic;
        } attenuation; // Used for Point and Spot Lights only
        // This specifies the inner and outer cone of the spot light.
        // The light power is 0 outside the outer cone, the light power is full inside the inner cone.
        // The light power is interpolated in between the inner and outer cone.
        struct {
            float inner, outer;
        } spot_angle; // Used for Spot Lights only


        // The ID of this component type is "Light"
        static std::string getID() { return "Light"; }

        // Deserialize light data from JSON
        void deserialize(const nlohmann::json& data) override;
        virtual ~LightComponent() = default;
    };

}
