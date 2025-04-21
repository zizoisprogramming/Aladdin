#include "generate.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads linearVelocity & angularVelocity from the given json object
    void GenerateComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        hide = data.value("hide", false);
    }
}