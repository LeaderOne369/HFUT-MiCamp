#include <GL/glut.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

enum class LightState
{
    RED,
    GREEN,
    YELLOW
};

LightState state = LightState::RED;
int timeCnt = 10;

void changeLight(int value)
{
    switch (state)
    {
    case LightState::RED:
        state = LightState::GREEN;
        timeCnt = 8;
        cout << "红灯结束，切换为绿灯" << endl;
        break;
    case LightState::GREEN:
        state = LightState::YELLOW;
        timeCnt = 2;
        cout << "绿灯结束，切换为黄灯" << endl;
        break;
    case LightState::YELLOW:
        state = LightState::RED;
        timeCnt = 10;
        cout << "黄灯结束，切换为红灯" << endl;
        break;
    }
    glutTimerFunc(timeCnt * 1000, changeLight, 0);
    glutPostRedisplay();
}

void drawCircle(float x, float y, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 100; i++)
    {
        float angle = 2.0f * 3.1415926f * float(i) / float(100);
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-0.2f, 0.5f);
    glVertex2f(0.2f, 0.5f);
    glVertex2f(0.2f, -0.5f);
    glVertex2f(-0.2f, -0.5f);
    glEnd();

    if (state == LightState::RED)
        glColor3f(1.0f, 0.0f, 0.0f);
    else
        glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(0.0f, 0.3f, 0.1f);

    if (state == LightState::YELLOW)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(0.0f, 0.0f, 0.1f);

    if (state == LightState::GREEN)
        glColor3f(0.0f, 1.0f, 0.0f);
    else
        glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(0.0f, -0.3f, 0.1f);

    glutSwapBuffers();
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("红绿灯");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(timeCnt * 1000, changeLight, 0);
    glutMainLoop();
    return 0;
}