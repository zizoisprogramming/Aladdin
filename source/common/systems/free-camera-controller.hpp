#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"
#include "../components/collision.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <fstream>
#include <iomanip> 

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked
        float bound_y = 4.0f;
        float lower_bound = 0.0f;
        std::string config_path = "config/generate.jsonc";
    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        void deSerializeSystem(World* world) {
            std::ifstream file_in(config_path);
            if(!file_in){
                std::cerr << "Couldn't open file: " << config_path << std::endl;
                return;
            }
            nlohmann::json app_config = nlohmann::json::parse(file_in, nullptr, true, true);
            file_in.close();
            bound_y = app_config["bound_y"];

            for(auto entity : world->getEntities()) {
                if (entity->name == "aladdin") {
                    CollisionComponent* collision = entity->getComponent<CollisionComponent>();
                    glm::mat4 player_matrix = entity->getLocalToWorldMatrix();
                    glm::vec3 position = glm::vec3(player_matrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 halfSize2 = glm::vec3(collision->x, collision->y, collision->z) * 0.5f;
                    lower_bound = halfSize2.y;
                }
            }
        }

        void checkEnd(World* world) {
            glm::vec3 position;
    
            // Find Aladdin
            for(auto entity : world->getEntities()){
                if (entity->name == "aladdin") {
                    position = entity->localTransform.position;
                    glm::mat4 player_matrix = entity->getLocalToWorldMatrix();
                    position = glm::vec3(player_matrix * glm::vec4(0, 0, 0, 1));
                }
            }
            
            // Check against villains
            bool villainFound = false;
            for(auto entity : world->getEntities()) {
                if (entity->name == "villain") {
                    glm::mat4 player_matrix = entity->getLocalToWorldMatrix();
                    if (position.z - 1 <= glm::vec3(player_matrix * glm::vec4(0, 0, 0, 1)).z) {
                        app->changeState("won");
                    }
                }
            } 
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        glm::vec3 update(World* world, float deltaTime, int last_num) {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if(camera && controller) break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if(!(camera && controller)) return {0, 0, 0};
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity* entity = camera->getOwner();

            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked){
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
            // If the left mouse button is released, we unlock and unhide the mouse.
            } else if(!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked) {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            // We get a reference to the entity's position and rotation
            glm::vec3& position = entity->localTransform.position;
            glm::vec3& rotation = entity->localTransform.rotation;

            
            // We update the camera fov based on the mouse wheel scrolling amount
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            camera->fovY = fov;

            // Get the camera model matrix
            glm::mat4 matrix = entity->localTransform.toMat4();

            // Calculate front, up, right directions
            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                    up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)), 
                    right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            // ====== NEW: Project front onto XZ plane (horizontal movement only) ======
            glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
            // ========================================================================

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            if(app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)) 
                current_sensitivity *= controller->speedupFactor;

            // Use horizontalFront instead of front for W/S movement
            if(app->getKeyboard().isPressed(GLFW_KEY_W)) 
                position += horizontalFront * (deltaTime * current_sensitivity.z);
            // if(app->getKeyboard().isPressed(GLFW_KEY_S)) position -= horizontalFront * ... 

            // Q/E (up/down) remains unchanged if you want vertical movement
            if(position.y < bound_y && app->getKeyboard().isPressed(GLFW_KEY_Q)) position += up * (deltaTime * current_sensitivity.y);
            if(position.y > lower_bound && app->getKeyboard().isPressed(GLFW_KEY_E)) position -= up * (deltaTime * current_sensitivity.y);

            // A/D (left/right) remains unchanged
            if((last_num != 3) && app->getKeyboard().isPressed(GLFW_KEY_D)) {
                position += right * (deltaTime * current_sensitivity.x);
            }
            if((last_num != 2) && app->getKeyboard().isPressed(GLFW_KEY_A)) {
                position -= right * (deltaTime * current_sensitivity.x);
            }

            checkEnd(world);
            return position;
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

    };

}

/*

glm::vec3 update(World* world, float deltaTime, int last_num) {
    // ... [previous code remains the same until the matrix calculation]

    

    // ... [rest of the function remains the same]
}
*/