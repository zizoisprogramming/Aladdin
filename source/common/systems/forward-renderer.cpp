#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include<fstream>
#include<algorithm>
#include<unordered_set>
#define MAX_LIGHTS 8 // Maximum number of lights supported

namespace our {

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json& config){
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if(config.contains("sky")){
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));
            
            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram* skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();
            
            //TODO: (Req 10) Pick the correct pipeline state to draw the sky
            // Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            // We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.faceCulling.enabled = 1;
            skyPipelineState.depthTesting.enabled= 1;
            skyPipelineState.faceCulling.culledFace = GL_FRONT; // cull the front cause this is a sky we want to see the back
            skyPipelineState.depthTesting.function = GL_LEQUAL; // will avoid fighting since will be drawm if equal since z of sky = 1

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D* skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky 
            Sampler* skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material 
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }


        if(config.contains("postprocessEffects")) {
            // === Framebuffer Setup ===
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
        
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
        
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Unbind
        
            // === Vertex Array for Post-process Quad ===
            glGenVertexArrays(1, &postProcessVertexArray);
        
            // === Sampler ===
            Sampler* postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
            // === Load Shaders from Config ===
            auto effects = config["postprocessEffects"];
            for (auto& [effectName, shaderPath] : effects.items()) {
                ShaderProgram* shader = new ShaderProgram();
                shader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
                shader->attach(shaderPath.get<std::string>(), GL_FRAGMENT_SHADER);
                shader->link();
                postprocessShaders[effectName] = shader;
            }
        
            // === Create Material ===
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            postprocessMaterial->pipelineState.depthMask = false;
        
            // Set default shader (e.g., "normal")
            if (postprocessShaders.count("normal")) {
                currentEffect = "normal";
                postprocessMaterial->shader = postprocessShaders["normal"];
            } else {
                currentEffect = effects.begin().key();
                postprocessMaterial->shader = postprocessShaders[currentEffect];
            }
        }
    }

    void ForwardRenderer::destroy(){
        // Delete all objects related to the sky
        if(skyMaterial){
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if(postprocessMaterial){
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World* world){
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent* camera = nullptr;
        //light component array
        //??
        opaqueCommands.clear();
        transparentCommands.clear();
        for(auto entity : world->getEntities()){
            // If we hadn't found a camera yet, we look for a camera in this entity
            if(!camera) camera = entity->getComponent<CameraComponent>();
            //if this entity has a light component
            if (auto light = entity->getComponent<LightComponent>(); light) {
                lights.push_back(light);
            }
            // If this entity has a mesh renderer component
            if(auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer){
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                
                // if it is transparent, we add it to the transparent commands list
                if(command.material->transparent){
                    transparentCommands.push_back(command);
                } else {
                // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if(camera == nullptr) return;

        //TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
       
        auto world_matrix = camera->getOwner()->getLocalToWorldMatrix(); 
        glm::vec3 eye = world_matrix * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = world_matrix * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);
        
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second) {
             return glm::dot(cameraForward, first.center) > glm::dot(cameraForward, second.center); 
        });

        //TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        //TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, windowSize.x, windowSize.y);
        
        //TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        
        //TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        

        // If there is a postprocess material, bind the framebuffer
        if(postprocessMaterial){
            //TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        }

        //TODO: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //



    

        //TODO: (Req 9) Draw all the opaque commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for(auto& command : opaqueCommands){
                command.material->setup();
                ShaderProgram* shader = command.material->shader;
                shader->use();
                command.material->shader->set("transform", VP * command.localToWorld);
                //for lighting shaders
                command.material->shader->set("object_to_world", command.localToWorld);
                command.material->shader->set("object_to_world_inv_transpose", glm::transpose(glm::inverse(command.localToWorld)));

                shader->set("view_projection", VP);
                shader->set("model", command.localToWorld);
                shader->set("camera_position", eye); // camera world position
                shader->set("ambient_light", ambientLight);
                std::cout <<"ambient light: " << ambientLight.x << " " << ambientLight.y << " " << ambientLight.z << std::endl;
                shader->set("light_count", static_cast<int>(lights.size()));
                
                // Set all light uniforms
            // only if material is lit compare with dynamic_cast
            if (auto litMaterial = dynamic_cast<LitMaterial*>(command.material)) {
               
                for (int i = 0; i < lights.size() && i < MAX_LIGHTS; i++) {
                    auto* light = lights[i];
                    std::string prefix = "lights[" + std::to_string(i) + "].";
            
                    glm::mat4 lightMatrix = light->getOwner()->getLocalToWorldMatrix();
                    glm::vec3 position = glm::vec3(lightMatrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 direction = glm::normalize(glm::vec3(lightMatrix * glm::vec4(0, 0, -1, 0)));
        
                    // Set type
                    int typeInt = 0;
                    switch (light->type) {
                        case LightComponent::LightType::Directional: typeInt = 0; break;
                        case LightComponent::LightType::Point:       typeInt = 1; break;
                        case LightComponent::LightType::Spot:        typeInt = 2; break;
                    }
        
                    // Set shader uniforms
                    shader->set(prefix + "type", typeInt);
                    shader->set(prefix + "color", light->color);
                    shader->set(prefix + "position", position);
                    shader->set(prefix + "direction", direction);
                    shader->set(prefix + "attenuation", glm::vec3{
                        light->attenuation.constant,
                        light->attenuation.linear,
                        light->attenuation.quadratic
                    });
                    shader->set(prefix + "cone_angles", glm::vec2{
                        light->spot_angle.inner,
                        light->spot_angle.outer
                    });
                }
            }
        
                command.mesh->draw();
           
        }
        
        // If there is a sky material, draw the sky
        if(this->skyMaterial){
            //TODO: (Req 10) setup the sky material
            skyMaterial->setup();
            //TODO: (Req 10) Get the camera position
            glm::vec3 cameraPosition = eye;
            //TODO: (Req 10) Create a model matrix for the sky such that it always follows the camera (sky sphere center = camera position)
            our::Transform skyTransform;
            skyTransform.position = cameraPosition;
            glm::mat4 modelMatrix = skyTransform.toMat4();
            //TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            // We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f
            );
            //TODO: (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * modelMatrix);    // vp is camera projection matrix
            //TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }
        //TODO: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto& command : transparentCommands){
            command.material->setup();
            command.material->shader->set("transform", VP * command.localToWorld);
            command.mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if(postprocessMaterial){
            //TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            if (shaking && (glfwGetTime() - shakeStartTime > shakeDuration)) {
                currentEffect = "normal";
                shaking = false;
            }
        
            auto it = postprocessShaders.find(currentEffect);
            if(it != postprocessShaders.end()){
                ShaderProgram* shader = it->second;
                postprocessMaterial->shader = shader;
                shader->use();
                shader->set("time", float(glfwGetTime()));
            }
            
            //TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            glBindVertexArray(this->postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

}