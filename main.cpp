#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "Shader.h"
#include "Sphere.h"
#include "Camera.h"
#include <iostream>
#include <vector>

// Globals
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera(glm::vec3(0.0f, 30.0f, -450.0f));


// Orbit VAOs and VBOs
unsigned int earthOrbitVAO, earthOrbitVBO;
unsigned int moonOrbitVAO, moonOrbitVBO;

// Background quad VAO and VBO
unsigned int backgroundVAO, backgroundVBO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset == 1)
        camera.ProcessKeyboard(SCROLL_FORWARD, deltaTime);
    else
        camera.ProcessKeyboard(SCROLL_BACKWARD, deltaTime);
}
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    return textureID;
}

void setupOrbit(float radius, unsigned int& vao, unsigned int& vbo, unsigned int segments = 100) {
    std::vector<float> vertices;
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void drawOrbit(Shader& shader, glm::vec3 center, glm::vec3 color, unsigned int vao) {
    shader.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, center);
    shader.setMat4("model", model);
    shader.setVec3("color", color);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_LOOP, 0, 101);
    glBindVertexArray(0);
}

void setupBackgroundQuad() {
    float vertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // Top-right
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom-right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f  // Bottom-left
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);
    glBindVertexArray(backgroundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void drawBackground(Shader& shader, unsigned int textureID) {
    shader.Use();
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setBool("useTexture", true);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    glBindVertexArray(backgroundVAO);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Simple Solar System", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    glfwSetScrollCallback(window, scroll_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0f); // Consistent line width for orbits

    Shader shader("simpleVS.vs", "simpleFS.fs");
    Sphere sun(100.0f, 72, 36);
    Sphere earth(35.0f, 36, 18);
    Sphere moon(10.0f, 36, 18);

    unsigned int tex_sun = loadTexture("resources/planets/c.jpg");
    unsigned int tex_earth = loadTexture("resources/planets/earth2k.jpg");
    unsigned int tex_moon = loadTexture("resources/planets/2k_moon.jpg");
    unsigned int tex_background = loadTexture("resources/A.jpg");

    // Setup orbit geometry
    setupOrbit(300.0f, earthOrbitVAO, earthOrbitVBO); // Earth's orbit
    setupOrbit(50.0f, moonOrbitVAO, moonOrbitVBO); // Moon's orbit

    // Setup background quad
    setupBackgroundQuad();


    glm::vec3 center = glm::vec3(0.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw background texture
        drawBackground(shader, tex_background);
        shader.Use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 1280.0f / 720.0f, 0.1f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // SUN
        glBindTexture(GL_TEXTURE_2D, tex_sun);
        glm::mat4 model_sun = glm::mat4(1.0f);
        model_sun = glm::translate(model_sun, center);
        model_sun = glm::rotate(model_sun, currentFrame * glm::radians(10.0f), glm::vec3(0, 1, 0));
        shader.setMat4("model", model_sun);
        shader.setBool("useTexture", true);
        sun.Draw();

        // EARTH
        glBindTexture(GL_TEXTURE_2D, tex_earth);
        float eX = sin(currentFrame * 0.5f) * 300.0f;
        float eZ = cos(currentFrame * 0.5f) * 300.0f;
        glm::vec3 earthPos = glm::vec3(eX, 0.0f, eZ);
        glm::mat4 model_earth = glm::mat4(1.0f);
        model_earth = glm::translate(model_earth, earthPos);
        model_earth = glm::rotate(model_earth, currentFrame * glm::radians(50.0f), glm::vec3(0, 1, 0));
        shader.setMat4("model", model_earth);
        shader.setBool("useTexture", true);
        earth.Draw();

        // MOON
        glBindTexture(GL_TEXTURE_2D, tex_moon);
        float mX = sin(currentFrame * 4.0f) * 50.0f;
        float mZ = cos(currentFrame * 4.0f) * 50.0f;
        glm::vec3 moonPos = earthPos + glm::vec3(mX, 0.0f, mZ);
        glm::mat4 model_moon = glm::mat4(1.0f);
        model_moon = glm::translate(model_moon, moonPos);
        model_moon = glm::rotate(model_moon, currentFrame * glm::radians(30.0f), glm::vec3(0, 1, 0));
        shader.setMat4("model", model_moon);
        shader.setBool("useTexture", true);
        moon.Draw();

        // Draw Earth's orbit (white)
        shader.setBool("useTexture", false);
        drawOrbit(shader, center, glm::vec3(1.0f, 1.0f, 1.0f), earthOrbitVAO);

        // Draw Moon's orbit (white)
        drawOrbit(shader, earthPos, glm::vec3(1.0f, 1.0f, 1.0f), moonOrbitVAO);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &earthOrbitVAO);
    glDeleteBuffers(1, &earthOrbitVBO);
    glDeleteVertexArrays(1, &moonOrbitVAO);
    glDeleteBuffers(1, &moonOrbitVBO);
    glDeleteVertexArrays(1, &backgroundVAO);
    glDeleteBuffers(1, &backgroundVBO);
    
    glfwTerminate();
    return 0;
}