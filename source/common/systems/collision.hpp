#pragma once

#include "../ecs/world.hpp"
#include "../components/collision.hpp"
#include "../components/camera.hpp"


#include <ecs/entity.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class CollisionSystem {
    public:
    
        bool safeToMove(World* world) {
            glm::vec3 position;
            int left_bound = 0;
            int right_bound = 0;

            // Find Aladdin
            for(auto entity : world->getEntities()) {
                if (entity->name == "aladdin") {
                    CollisionComponent* collision = entity->getComponent<CollisionComponent>();
                    position = entity->localTransform.position;
                    glm::mat4 player_matrix = entity->getLocalToWorldMatrix();
                    position = glm::vec3(player_matrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 halfSize2 = glm::vec3(collision->x, collision->y, collision->z) * 0.5f;
                    left_bound = position.x - halfSize2.x;
                    right_bound = position.x + halfSize2.x;
                }
            }
            
            // Check against left and right wall
            for(auto entity : world->getEntities()) {
                if (entity->name == "left_wall") {
                    glm::mat4 matrix = entity->getLocalToWorldMatrix();
                    if (left_bound <= glm::vec3(matrix * glm::vec4(0, 0, 0, 1)).x) {
                        return false;
                    }
                }
                if (entity->name == "right_wall") {
                    glm::mat4 matrix = entity->getLocalToWorldMatrix();
                    if (right_bound >= glm::vec3(matrix * glm::vec4(0, 0, 0, 1)).x) {
                        return false;
                    }
                }
            } 

            return true;
        }
       
        bool check_collision(glm::vec3& pos1, glm::vec3& pos2, 
                    CollisionComponent* collision1, CollisionComponent* collision2) {

            glm::vec3 halfSize2 = glm::vec3(collision2->x, collision2->y, collision2->z) * 0.5f;

            glm::vec3 min1 = pos1 - glm::vec3(0.5 * collision1->x, 0, 0.5 * collision1->z);
            glm::vec3 max1 = pos1 + glm::vec3(0.5 * collision1->x, collision1->y, 0.5 * collision1->z);
            glm::vec3 min2 = pos2 - halfSize2;
            glm::vec3 max2 = pos2 + halfSize2;

            return ((max2.x >= min1.x && max2.x <= max1.x) || (min2.x <= max1.x && min2.x >= min1.x)) &&
                    ((max2.y >= min1.y && max2.y <= max1.y) || (min2.y <= max1.y && min2.y >= min1.y)) &&
                    ((max2.z >= min1.z && max2.z <= max1.z) || (min2.z <= max1.z && min2.z >= min1.z));
        }

        // This should be called every frame to update all entities containing a MovementComponent. 
        int update(World* world, float deltaTime) {
            our::Entity* player = nullptr;

            for (auto entity : world->getEntities()){
                CollisionComponent* collision = entity->getComponent<CollisionComponent>();
                if(collision && entity->parent && entity->parent->getComponent<CameraComponent>())
                    player = entity;
            }

            if (!player) return false;

            for(auto entity : world->getEntities()){
                if(entity->getComponent<CollisionComponent>() && entity != player) { 
                    glm::mat4 matrix = entity->getLocalToWorldMatrix();
                    glm::mat4 player_matrix = player->getLocalToWorldMatrix();
                    glm::vec3 pos1 = glm::vec3(player_matrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 pos2 = glm::vec3(matrix * glm::vec4(0, 0, 0, 1));
                    
                    CollisionComponent* player_collision = player->getComponent<CollisionComponent>();
                    CollisionComponent* collision = entity->getComponent<CollisionComponent>();
                    if (check_collision(pos1, pos2, player_collision, collision))
                        return 1;
                }
            }

            if (!safeToMove(world))
                return 2;
            return 0;
        }

    };

}
