#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

using namespace std;

const char *vShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
out vec3 fragColor;
void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = color;
})";

const char *fShaderSrc = R"(
#version 330 core
in vec3 fragColor;
out vec4 color;
void main() {
    color = vec4(fragColor, 1.0);
})";

float verts[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f};

GLuint VBO, VAO;

void mouseCB(GLFWwindow *win, double xpos, double ypos)
{
    int w, h;
    glfwGetWindowSize(win, &w, &h);
    float x = (2.0f * xpos) / w - 1.0f;
    float y = 1.0f - (2.0f * ypos) / h;

    if (x >= -0.5f && x <= 0.5f && y >= -0.5f && y <= 0.5f)
    {
        float r = (x + 0.5f);
        float g = (y + 0.5f);
        float b = 1.0f - r - g;
        stringstream colorInfo;
        colorInfo << "RGB: (" << r << ", " << g << ", " << b << ")";
        glfwSetWindowTitle(win, colorInfo.str().c_str());
    }
}

int main()
{
    if (!glfwInit())
    {
        cerr << "初始化GLFW失败\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(800, 600, "彩色三角形", nullptr, nullptr);
    if (!win)
    {
        cerr << "创建窗口失败\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(win);
    glfwSetCursorPosCallback(win, mouseCB);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cerr << "初始化GLEW失败\n";
        return -1;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSrc, nullptr);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSrc, nullptr);
    glCompileShader(fShader);

    GLuint shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vShader);
    glAttachShader(shaderProg, fShader);
    glLinkProgram(shaderProg);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    while (!glfwWindowShouldClose(win))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProg);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProg);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}