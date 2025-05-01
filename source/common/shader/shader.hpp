#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<iostream>

namespace our {

    class ShaderProgram {

    private:
        //Shader Program Handle (OpenGL object name)
        GLuint program;

    public:
        ShaderProgram(){
            //TODO: (Req 1) Create A shader program
            program = glCreateProgram();
            if (program == 0) {
                std::cerr << "Failed to create shader program" << std::endl;
            }
        }
        /**
         * Destructor for ShaderProgram
         *
         * Deletes the shader program OpenGL object on destruction of the
         * ShaderProgram object
         */
        ~ShaderProgram(){
            //TODO: (Req 1) Delete a shader program
            if (program != 0) 
                glDeleteProgram(program);
            else
                std::cerr << "Failed to delete shader program" << std::endl;
                    
        }

        bool attach(const std::string &filename, GLenum type) const;

        bool link() const;

        void use() { 
            glUseProgram(program);
        }

        GLuint getUniformLocation(const std::string &name) {
            //TODO: (Req 1) Return the location of the uniform with the given name
            GLuint loc = glGetUniformLocation(program, name.c_str());
            if (loc == -1) {
                // std::cerr << "Uniform " << name << " not found" << std::endl;
            }
            return static_cast<GLuint>(loc);
        }

        void set(const std::string &uniform, GLfloat value) {
            //TODO: (Req 1) Send the given float value to the given uniform
            GLuint loc = getUniformLocation(uniform);

            glUniform1f(loc, value);

        }

        void set(const std::string &uniform, GLuint value) {
            //TODO: (Req 1) Send the given unsigned integer value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniform1ui(loc, value);
        }

        void set(const std::string &uniform, GLint value) {
            //TODO: (Req 1) Send the given integer value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniform1i(loc, value);
        }

        void set(const std::string &uniform, glm::vec2 value) {
            //TODO: (Req 1) Send the given 2D vector value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniform2fv(loc, 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::vec3 value) {
            //TODO: (Req 1) Send the given 3D vector value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniform3fv(loc, 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::vec4 value) {
            //TODO: (Req 1) Send the given 4D vector value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniform4fv(loc, 1, glm::value_ptr(value));
        }

        void set(const std::string &uniform, glm::mat4 matrix) {
            //TODO: (Req 1) Send the given matrix 4x4 value to the given uniform
            GLuint loc = getUniformLocation(uniform);
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
        }

        //TODO: (Req 1) Delete the copy constructor and assignment operator.
        //Question: Why do we delete the copy constructor and assignment operator?

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator =(const ShaderProgram&) = delete;
    };

}

#endif