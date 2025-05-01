#include "light.hpp"
#include<iostream>

namespace our {

    // Deserialize light data from JSON
    void LightComponent::deserialize(const nlohmann::json& data) {
        if (data.contains("color")) { //light color
            color = glm::vec3(data["color"][0], data["color"][1], data["color"][2]);
        }
        
        if (data.contains("lightType")) {
            
            std::string typeStr = data["lightType"];
            if (typeStr == "Point") {
                type = LightType::Point;
            } else if (typeStr == "Directional") {
                type = LightType::Directional;
            } else if (typeStr == "Spot") {
                type = LightType::Spot;
            }
        }
        if (data.contains("attenuation")) { 
            attenuation.constant = data["attenuation"]["constant"];
            attenuation.linear = data["attenuation"]["linear"];
            attenuation.quadratic = data["attenuation"]["quadratic"];
        }
        if (data.contains("spot_angle")) { 
            spot_angle.inner = data["spot_angle"]["inner"];
            spot_angle.outer = data["spot_angle"]["outer"];
        }
        if (data.contains("direction")) { 
            direction = glm::vec3(data["direction"][0], data["direction"][1], data["direction"][2]);
        }
        if(data.contains("lightPosition")){ { 
            position = glm::vec3(data["lightPosition"][0], data["lightPosition"][1], data["lightPosition"][2]);
        }
    }

    }

}
