#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "FastNoiseLite/FastNoiseLite.h"
#include "shader.h"
#include "camera.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

// Constants
const unsigned int WIDTH = 800, HEIGHT = 600;
const int TERRAIN_WIDTH = 100, TERRAIN_HEIGHT = 100;

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

std::vector<float> generateHeightmap() {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.05f);

    std::vector<float> heightmap(TERRAIN_WIDTH * TERRAIN_HEIGHT);

    for (int y = 0; y < TERRAIN_HEIGHT; ++y) {
        for (int x = 0; x < TERRAIN_WIDTH; ++x) {
            heightmap[x + y * TERRAIN_WIDTH] = noise.GetNoise((float)x, (float)y);
        }
    }

    return heightmap;
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

std::vector<Vertex> generateTerrainVertices(const std::vector<float>& heightmap) {
    std::vector<Vertex> vertices(TERRAIN_WIDTH * TERRAIN_HEIGHT);

    for (int y = 0; y < TERRAIN_HEIGHT; ++y) {
        for (int x = 0; x < TERRAIN_WIDTH; ++x) {
            float height = heightmap[x + y * TERRAIN_WIDTH];

            Vertex vertex;
            vertex.position = glm::vec3(x, height * 10.0f, y);  // Scale height for visibility
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);        // Placeholder normal
            vertex.color = glm::vec3(0.2f, 0.7f, 0.3f);         // Grass-like color

            vertices[x + y * TERRAIN_WIDTH] = vertex;
        }
    }

    return vertices;
}

std::vector<unsigned int> generateTerrainIndices() {
    std::vector<unsigned int> indices;
    indices.reserve((TERRAIN_WIDTH - 1) * (TERRAIN_HEIGHT - 1) * 6);

    for (int y = 0; y < TERRAIN_HEIGHT - 1; ++y) {
        for (int x = 0; x < TERRAIN_WIDTH - 1; ++x) {
            unsigned int bottomLeft = x + y * TERRAIN_WIDTH;
            unsigned int bottomRight = (x + 1) + y * TERRAIN_WIDTH;
            unsigned int topLeft = x + (y + 1) * TERRAIN_WIDTH;
            unsigned int topRight = (x + 1) + (y + 1) * TERRAIN_WIDTH;

            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topLeft);

            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
            indices.push_back(topRight);
        }
    }

    return indices;
}

void setupTerrainBuffers(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, GLuint& VAO, GLuint& VBO, GLuint& EBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void renderTerrain(GLuint VAO, size_t indexCount) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Main function
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "FastNoise Terrain", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glClearColor(0.1f, 0.15f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Shader
    Shader shader("C:\\Users\\xiaoy\\OneDrive\\Desktop\\Procedural Rendering\\shaders\\vertex_shader.glsl",
              "C:\\Users\\xiaoy\\OneDrive\\Desktop\\Procedural Rendering\\shaders\\fragment_shader.glsl");

    // Generate terrain
    std::vector<float> heightmap = generateHeightmap();
    std::vector<Vertex> vertices = generateTerrainVertices(heightmap);
    std::vector<unsigned int> indices = generateTerrainIndices();

    // Print vertex data for debugging
    for (const auto& vertex : vertices) {
        std::cout << "Vertex Position: " << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << std::endl;
    }

    // Setup OpenGL buffers
    GLuint VAO, VBO, EBO;
    setupTerrainBuffers(vertices, indices, VAO, VBO, EBO);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 50.0f, 150.0f), glm::vec3(50.0f, 0.0f, 50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 300.0f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        renderTerrain(VAO, indices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Callback functions and additional helpers
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}