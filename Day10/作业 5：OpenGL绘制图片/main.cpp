#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// 初始化 OpenGL 光照
void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPos[] = {0.0f, 0.0f, 3.0f, 1.0f};     // 光源位置
    GLfloat lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};   // 白色光
    GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f}; // 环境光

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

// 初始化 OpenGL
void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    setupLighting();
}

// 绘制立方体（带法线）
void drawCube(float s)
{
    float hs = s / 2.0f;
    glBegin(GL_QUADS);

    // 前面
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-hs, -hs, hs);
    glVertex3f(hs, -hs, hs);
    glVertex3f(hs, hs, hs);
    glVertex3f(-hs, hs, hs);

    // 后面
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hs, -hs, -hs);
    glVertex3f(-hs, hs, -hs);
    glVertex3f(hs, hs, -hs);
    glVertex3f(hs, -hs, -hs);

    // 上面
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hs, hs, -hs);
    glVertex3f(-hs, hs, hs);
    glVertex3f(hs, hs, hs);
    glVertex3f(hs, hs, -hs);

    // 下面
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hs, -hs, -hs);
    glVertex3f(hs, -hs, -hs);
    glVertex3f(hs, -hs, hs);
    glVertex3f(-hs, -hs, hs);

    // 右面
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hs, -hs, -hs);
    glVertex3f(hs, hs, -hs);
    glVertex3f(hs, hs, hs);
    glVertex3f(hs, -hs, hs);

    // 左面
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hs, -hs, -hs);
    glVertex3f(-hs, -hs, hs);
    glVertex3f(-hs, hs, hs);
    glVertex3f(-hs, hs, -hs);

    glEnd();
}

// 显示回调函数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 绘制绿色立方体（带旋转）
    glPushMatrix();
    glTranslatef(0.0, 0.0, -6.0);
    glRotatef(45.0f, 1.0f, 1.0f, 0.0f);
    glColor3f(0.0, 1.0, 0.0);
    drawCube(1.0);
    glPopMatrix();
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "初始化 GLFW 失败" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Shapes", NULL, NULL);
    if (!window)
    {
        std::cerr << "创建 GLFW 窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "初始化 GLEW 失败" << std::endl;
        return -1;
    }

    init();

    glViewport(0, 0, 800, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}