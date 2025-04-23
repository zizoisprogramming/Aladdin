#include "collision.hpp"
#include "../asset-loader.hpp"

namespace our {
    // Receives the mesh & material from the AssetLoader by the names given in the json object
    void CollisionComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        x = data.value("x", x);
        y = data.value("y", y);
        z = data.value("z", z);
    }
}