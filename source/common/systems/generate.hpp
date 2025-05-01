#pragma once

#include "../ecs/world.hpp"
#include "../components/generate.hpp"
#include "../ecs/entity.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <fstream>
#include <random>



namespace our
{
    class GenerateSystem {
        float till_now = 0;
        float threshold = 1.0;
    public:

        void resetParameters() {
            till_now = 0;
            threshold = 1.0;
        }
        void update(World* world, float deltaTime, bool level_up = false) {
            till_now += deltaTime;
            bool first = false;
            for (auto entity : world->getEntities()) {
                GenerateComponent* generate = entity->getComponent<GenerateComponent>();
                if (generate) {
                    first = true;
                    if (generate->hide) {
                        entity->localTransform.position = glm::vec3(0, 0, -3);
                        generate->hide = false;
                        first = false;
                    }
                }
            }
            if (level_up)
                threshold -= 0.4;

            if (first && till_now >= threshold) {
                till_now = 0;

                Entity* newEntity = world->add(); 
                newEntity->parent = nullptr;

                std::random_device rd;  // Obtain a random seed
                std::mt19937 gen(rd()); // Standard Mersenne Twister engine
                std::uniform_real_distribution<float> distX(-5.0f, 5.0f); // X range: [-5, 5]
                std::uniform_real_distribution<float> distY(0.0f, 4.0f);  // Y range: [0, 4]
                
                float randomX = distX(gen);
                float randomY = distY(gen);

                nlohmann::json data = {
                    {"position", {randomX, randomY, -3}}, // change villain
                    {"rotation", {0, 0, 0}},
                    {"scale", {0.25, 0.25, 0.25}},
                    {"hide", false},
                    {"components", {
                        {
                            {"type", "Generate"}
                        },
                        {
                            {"type", "Mesh Renderer"},
                            {"mesh", "ball"},
                            {"material", "fire_ball"}
                        },
                        {
                            {"type", "Movement"},
                            {"linearVelocity", {0, 0, 3}},
                            {"angularVelocity", {0, 0, 0}}
                        },
                        {
                            {"type", "Collision"},
                            {"x", 0.25},
                            {"y", 0.25},
                            {"z", 0.25}
                        }
                    }}
                };
                newEntity->deserialize(data); 
            }

        }
    };
}