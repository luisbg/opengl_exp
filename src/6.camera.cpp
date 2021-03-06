#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <SDL.h>
#include <SDL_image.h>

// screen
const GLuint WIDTH = 800, HEIGHT = 600;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// mouse data
bool firstMouse = true;
// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a
// direction vector pointing to the right so we initially rotate a bit to the
// left.
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// process all input
// query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    float cameraSpeed = 5.0f * deltaTime;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // avoid jumping when first receiving focus of mouse cursor
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// whenever the mouse moves, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
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
    GLuint VBO_T, VAO_T, EBO_T, VBO_F, VAO_F, EBO_F;

    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Camera", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, processInput);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    Shader tetraShader("../src/6.camera.vs", "../src/6.camera.fs");
    Shader floorShader("../src/6.camera.vs", "../src/6.camera.fs2");

    GLfloat tetra_vertices[] = {
        // positions         // texture coords
        -0.5f, 0.5f, 0.0f,   0.0f, 0.0f,  // top left
        0.5f, 0.5f, 0.0f,    1.0f, 0.0f,  // top right
        0.0f, -0.5f, -0.5f,  0.0f, 1.0f,  // bottom
        0.0f, 0.5f, -1.0f,   1.0f, 1.0f   // back
    };

    GLfloat floor_vertices[] = {
        -100.0f, -0.5f, -100.0f,
        -100.0f, -0.5f,  100.0f,
         100.0f, -0.5f, -100.0f,
         100.0f, -0.5f,  100.0f
    };

    GLint tetra_indices[] = {
        0, 1, 2,   // front
        1, 2, 3,   // right
        0, 2, 3,   // left
        0, 1, 3    // back
    };

    GLint floor_indices[] = {
        0, 1, 2,   // left
        1, 2, 3,   // right
    };

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_Surface* tetra_surface = IMG_Load("../img/sky.jpg");
    if (tetra_surface)
    {
        GLenum texture_format;

        // Check that the image's width is a power of 2
        if ((tetra_surface->w & (tetra_surface->w - 1)) != 0)
            std::cout << "Image width is not a power of 2" << std::endl;

        // Also check if the height is a power of 2
        if ((tetra_surface->h & (tetra_surface->h - 1)) != 0)
            std::cout << "Image height is not a power of 2" << std::endl;

        // get the number of channels in the SDL surface
        GLint nOfColors = tetra_surface->format->BytesPerPixel;
        if (nOfColors == 4)     // contains an alpha channel
        {
            if (tetra_surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGBA;
            else
                texture_format = GL_BGRA;
        }
        else if (nOfColors == 3)     // no alpha channel
        {
            if (tetra_surface->format->Rmask == 0x000000ff)
                texture_format = GL_RGB;
            else
                texture_format = GL_BGR;
        }
        else
        {
            std::cout << "the image is not truecolor.." << std::endl;
        }

        std::cout << "Loaded sky image with size: " << tetra_surface->w << "," << tetra_surface->h << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tetra_surface->w, tetra_surface->h, 0,
                     texture_format, GL_UNSIGNED_BYTE, tetra_surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

    // Bind vertex array object, and set vertex buffer(s) and attribute pointer(s)
    glGenVertexArrays(1, &VAO_T);
    glGenBuffers(1, &VBO_T);
    glGenBuffers(1, &EBO_T);
    glGenVertexArrays(1, &VAO_F);
    glGenBuffers(1, &VBO_F);
    glGenBuffers(1, &EBO_F);

    glBindVertexArray(VAO_T);

    // Copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO_T);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetra_vertices), tetra_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_T);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetra_indices), tetra_indices, GL_STATIC_DRAW);

    // Then set the vertex attributes pointers
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Set floor
    glBindVertexArray(VAO_F);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_F);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_F);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floor_indices), floor_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Rendering commands here
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind our texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(tetraShader.ID, "ourTexture"), 0);

        glm::mat4 view;
        glm::mat4 projection;

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov),
                                      (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

        // Use our shader program when we want to render an object
        tetraShader.use();

        // Retrieve the matrix uniform locations
        GLint viewLoc  = glGetUniformLocation(tetraShader.ID, "view");
        GLint projectLoc = glGetUniformLocation(tetraShader.ID, "projection");

        // Pass them to the shaders
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO_T);

        GLint modelLoc;
        for(unsigned int i = 0; i < 4; i++)
        {
            // Create transformations
            glm::mat4 model;
            model = glm::translate(model, cubePositions[i]);
            modelLoc = glGetUniformLocation(tetraShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        }

        floorShader.use();
        glBindVertexArray(VAO_F);

        glm::mat4 model;
        modelLoc = glGetUniformLocation(floorShader.ID, "model");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap back buffer to front, and check events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Deallocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO_T);
    glDeleteBuffers(1, &EBO_T);
    glDeleteBuffers(1, &VBO_T);
    glDeleteVertexArrays(1, &VAO_F);
    glDeleteBuffers(1, &VBO_F);
    glDeleteBuffers(1, &EBO_F);

    glfwTerminate();

    return 0;
}
