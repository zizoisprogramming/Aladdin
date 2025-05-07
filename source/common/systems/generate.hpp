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
        std::string config_path = "config/generate.jsonc";
        float till_now = 0;
        float threshold = 1.0;
        float level_up_val = 0.4;
        float bound_x = 0.0;
        float bound_y = 4.0;
        float bound_y_low = 0.0;
    public:

        void deSerializeSystem() {
            std::ifstream file_in(config_path);
            if(!file_in){
                std::cerr << "Couldn't open file: " << config_path << std::endl;
                return;
            }
            nlohmann::json app_config = nlohmann::json::parse(file_in, nullptr, true, true);
            file_in.close();

            till_now = app_config.value("till_now", 0.0f);
            level_up_val = app_config.value("level_up_val", 0.37f);
            threshold = app_config.value("threshold", 1.0f);
            bound_y = app_config.value("bound_y", 4.0f);
            bound_x = app_config.value("bound_x", 5.0f);
            bound_y_low = app_config.value("bound_y_low", 1.0f);
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

            nlohmann::json randomObject = app_config["random"];
            randomObject["position"] = {randomX, randomY, app_config["generate_pos"]};

            return randomObject;
        }
        void update(World* world, float deltaTime, bool level_up = false) {
            till_now += deltaTime;

            if (level_up)
                threshold -= level_up_val;

            if (till_now >= threshold) {
                till_now = 0;

                Entity* newEntity = world->add(); 
                newEntity->parent = nullptr;

                std::random_device rd;  
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> distX(-bound_x, bound_x);
                std::uniform_real_distribution<float> distY(bound_y_low, bound_y);  
                
                float randomX = distX(gen);
                float randomY = distY(gen);

                nlohmann::json data = readGenerateConfig(randomX, randomY);
                newEntity->deserialize(data); 
            }

        }
    };
}