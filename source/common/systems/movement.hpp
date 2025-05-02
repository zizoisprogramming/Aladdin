#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    public:

        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime, int last_num) {
            // For each entity in the world
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();

                if(movement){
                    glm::vec3 newVelocity = movement->linearVelocity;
                    if (entity->name == "aladdin" && last_num == 2) {
                        newVelocity.x = 0;
                    }
                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    entity->localTransform.position += (deltaTime) * newVelocity;
                }
            }
        }

    };

}
