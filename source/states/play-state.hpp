#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/generate.hpp>
#include <systems/collision.hpp>
#include<fstream>
#include<systems/sound.hpp>

#include <asset-loader.hpp>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::GenerateSystem generateSystem;
    our::CollisionSystem collisionSystem;
    Sound playSound = Sound("C:/Users/Acer/Desktop/Graphics Project/Aladdin/assets/sounds/play.mp3", false);
    std::string config_path = "config/play-conf.jsonc";

    float initializationDelay = 2.0f;  
    bool initializationComplete = false;
    float levels[3] = {10, 7, 3};
    int curr_ind = 0;
    bool not_again = false;
    int last_num = 0;
    bool hitOnce = false;
    float hitDelayTimer = 0.0f;

    void deSerializeState() {
        std::ifstream file_in(config_path);
        if(!file_in){
            std::cerr << "Couldn't open file: " << config_path << std::endl;
            return;
        }
        nlohmann::json state_config = nlohmann::json::parse(file_in, nullptr, true, true);
        file_in.close();

        initializationComplete = state_config["initializationComplete"];
        initializationDelay = state_config["initializationDelay"];
        last_num = state_config["last_num"];
        curr_ind = state_config["curr_ind"];
        not_again = state_config["not_again"];
        levels[0] = state_config["levels"][0];
        levels[1] = state_config["levels"][1];
        levels[2] = state_config["levels"][2];
        hitOnce = state_config["hitOnce"];
        hitDelayTimer = state_config["hitDelayTimer"];

        generateSystem.deSerializeSystem();
        cameraController.deSerializeSystem(&world);
    }

    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
        }
    
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        deSerializeState();
        playSound.play(1);
        renderer.initialize(size, config["renderer"]);

    }

    bool level_up(glm::vec3& position) {
        bool flag = false;
        
        if (!not_again && position.z <= levels[curr_ind]){
            curr_ind = (curr_ind + 1) % 3;
            if (curr_ind == 0)
                not_again = true;
            else
                flag = true;
        }
        return flag;
    }

    void onDraw(double deltaTime) override {
        if (!initializationComplete) {
            initializationDelay -= (float)deltaTime;
            if (initializationDelay <= 0.0f) {
                initializationComplete = true;
            }
            return;  
        }
        movementSystem.update(&world, (float)deltaTime);
        glm::vec3 position = cameraController.update(&world, (float)deltaTime, last_num);

        bool flag = level_up(position);
        generateSystem.update(&world, (float)deltaTime, flag);
        int num = collisionSystem.update(&world, (float)deltaTime);
        last_num = num;
        hitDelayTimer += 1.0;
        if (num == 1) {
            if (hitOnce) {
                if (hitDelayTimer >= 100.0f) {
                    getApp()->changeState("lost");
                }
            } else {
                renderer.startShake();  
                hitDelayTimer = 0.0f;
            }
            hitOnce = true;
        }
        
        renderer.render(&world);
        auto& keyboard = getApp()->getKeyboard();
        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            getApp()->changeState("menu");
        }

        if(keyboard.justPressed(GLFW_KEY_2)){
            renderer.switchEffect("confuse");
        }
        if(keyboard.justPressed(GLFW_KEY_3)){
            renderer.switchEffect("normal");
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        playSound.stop();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }

};