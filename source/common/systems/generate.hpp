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
        std::string config_path = "config/generate.jsonc";
    public:

        void resetParameters() {
            till_now = 0;
            threshold = 1.0;
        }

        nlohmann::json readGenerateConfig(float randomX, float randomY) {
            std::ifstream file_in(config_path);
            if(!file_in){
                std::cerr << "Couldn't open file: " << config_path << std::endl;
                return -1;
            }
            // Read the file into a json object then close the file
            nlohmann::json app_config = nlohmann::json::parse(file_in, nullptr, true, true);
            file_in.close();

            app_config["position"] = {randomX, randomY, -3};

            return app_config;
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

                nlohmann::json data = readGenerateConfig(randomX, randomY);
                    
                newEntity->deserialize(data); 
            }

        }
    };
}