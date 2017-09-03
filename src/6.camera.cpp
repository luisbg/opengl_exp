#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// screen
const GLuint WIDTH = 800, HEIGHT = 600;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

// process all input
// query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    std::cout << "New window width: " << width << " and height: " << height << std::endl;
    glViewport(0, 0, width, height);
}

int main()
{
    GLuint VBO, VAO, EBO;

    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, processInput);

    glewExperimental = GL_TRUE;
    glewInit();

    int numAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
    std::cout << "Maximum num of vertex attributes supported: " << numAttributes << std::endl;

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Build our shader program
    Shader ourShader("../src/6.camera.vs", "../src/6.camera.fs");

    GLfloat triangle_vertices[] = {
        // positions         // texture coords
        -0.5f, 0.5f, 0.0f,   0.0f, 0.0f,  // top left
        0.5f, 0.5f, 0.0f,    1.0f, 0.0f,  // top right
        0.0f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom
        0.0f, 0.5f, -1.0f,   1.0f, 1.0f   // back
    };
    GLint indices[] = {
        0, 1, 2,   // front
        1, 2, 3,   // right
        0, 2, 3,   // left
        0, 1, 3    // back
    };

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int tex_width, tex_height, nrChannels;
    unsigned char *data = stbi_load("../img/sky.jpg", &tex_width, &tex_height, &nrChannels, 4);
    if (data)
    {
        std::cout << "Loaded sky image with size: " << tex_width << "," << tex_height << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

    // Bind vertex array object, and set vertex buffer(s) and attribute pointer(s)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Then set the vertex attributes pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 1.0f,  0.0f, -2.5f),
        glm::vec3(-1.0f,  0.0f, -2.5f),
        glm::vec3( 2.0f,  0.0f, -5.0f),
    };

    // We can set this to GL_LINE to use wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        // Rendering commands here
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind our texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(ourShader.ID, "ourTexture"), 0);

        // Use our shader program when we want to render an object
        ourShader.use();

        glm::mat4 view;
        glm::mat4 projection;

        float radius = 10.0f;
        float camX = sin(glfwGetTime()) * radius;
        float camZ = cos(glfwGetTime()) * radius;
        view = glm::lookAt(glm::vec3(camX, 1.0, camZ), glm::vec3(0.0, 0.0, 0.0),
                           glm::vec3(0.0, 1.0, 0.0));
        projection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

        // Retrieve the matrix uniform locations
        GLint viewLoc  = glGetUniformLocation(ourShader.ID, "view");
        GLint projectLoc = glGetUniformLocation(ourShader.ID, "projection");

        // Pass them to the shaders
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        for(unsigned int i = 0; i < 4; i++)
        {
            // Create transformations
            glm::mat4 model;

            model = glm::translate(model, cubePositions[i]);
            float angle = 10.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            GLint modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        }

        // Draw our triangle
        glBindVertexArray(0);

        // Swap back buffer to front, and check events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Deallocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();

    return 0;
}
