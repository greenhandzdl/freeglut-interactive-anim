/**
 * @file main.cpp
 * @brief 应用阶段：窗口创建、输入回调、主循环
 *
 * 将复杂的图形管线拆分到独立模块：
 *   math/    — Vec3, Mat4 数学工具
 *   geometry/— 网格生成（圆柱/球体/台体/SLERP 关节连接器）
 *   shaders/ — GLSL 着色器源码
 *   render/  — GL 初始化、编译/链接、每帧更新与绘制
 */
#include "common.h"
#include "render/render.h"
#include "math/math.h"

#include <iostream>
#include <algorithm>
#include <cstring>

#include <GL/glew.h>
#include <GL/freeglut.h>

// ============================================================
// 窗口设置
// ============================================================
static const int WIN_W = 1024;
static const int WIN_H = 768;
static const char* WIN_TITLE = "OpenGL 人物演示 — 顶点动画 + 曲面细分";

// ============================================================
// FreeGLUT 回调
// ============================================================
static void displayCallback() {
    renderScene();
}

static void idleCallback() {
    static int lastTime = 0;
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (lastTime == 0) lastTime = now;
    float dt = (now - lastTime) / 1000.0f;
    if (dt > 0.05f) dt = 0.05f;
    lastTime = now;

    updateAnimation(dt);
    glutPostRedisplay();
}

static void keyboardCallback(unsigned char key, int x, int y) {
    (void)x; (void)y;

    switch (key) {
        case 27:
#ifdef USE_FREEGLUT
            glutLeaveMainLoop();
#else
            exit(0);
#endif
            break;

        case ' ':
            if (g.jumpPhase <= 0.0f || g.jumpPhase >= 1.0f) {
                g.jumping = true;
                std::cout << "[跳跃] 起跳！" << std::endl;
            }
            break;

        case '+':
        case '=':
            g.tessLevel = std::min(g.tessLevel + 0.5f, 12.0f);
            std::cout << "[细分级别] " << g.tessLevel << std::endl;
            break;

        case '-':
        case '_':
            g.tessLevel = std::max(g.tessLevel - 0.5f, 1.0f);
            std::cout << "[细分级别] " << g.tessLevel << std::endl;
            break;

        default:
            break;
    }

    g.keys[key] = true;
}

static void keyboardUpCallback(unsigned char key, int x, int y) {
    (void)x; (void)y;
    g.keys[key] = false;

    if (key == ' ') {
        g.jumping = false;
    }
}

static void specialCallback(int key, int x, int y) {
    (void)x; (void)y;
    switch (key) {
        case GLUT_KEY_LEFT:
            g.cameraAngle -= 0.05f;
            break;
        case GLUT_KEY_RIGHT:
            g.cameraAngle += 0.05f;
            break;
        case GLUT_KEY_UP:
            g.cameraHeight = std::min(g.cameraHeight + 0.2f, 5.0f);
            break;
        case GLUT_KEY_DOWN:
            g.cameraHeight = std::max(g.cameraHeight - 0.2f, 1.0f);
            break;
    }
}

static void reshapeCallback(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
}

static void visibilityCallback(int visible) {
    if (visible == GLUT_VISIBLE) {
        memset(g.keys, 0, sizeof(g.keys));
    }
}

static void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            g.mouseDragging = true;
            g.mouseLastX = x;
            g.mouseLastY = y;
        } else {
            g.mouseDragging = false;
        }
    }
    if (button == 3) {
        g.cameraDist = std::max(g.cameraDist - 0.3f, 2.0f);
    }
    if (button == 4) {
        g.cameraDist = std::min(g.cameraDist + 0.3f, 15.0f);
    }
}

static void mouseMotionCallback(int x, int y) {
    if (g.mouseDragging) {
        int dx = x - g.mouseLastX;
        int dy = y - g.mouseLastY;
        g.cameraAngle -= dx * 0.008f;
        g.cameraHeight = std::max(1.0f, std::min(5.0f, g.cameraHeight + dy * 0.01f));
        g.mouseLastX = x;
        g.mouseLastY = y;
    }
}

static void printInfo() {
    std::cout << "================================================" << std::endl;
    std::cout << " OpenGL 人物演示 — 顶点动画 + 曲面细分" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "  操作说明:" << std::endl;
    std::cout << "  W/A/S/D  — 移动人物（相对于摄像机方向）" << std::endl;
    std::cout << "  SPACE    — 跳跃" << std::endl;
    std::cout << "  鼠标左键拖拽 — 旋转/升降摄像机" << std::endl;
    std::cout << "  鼠标滚轮 — 缩放摄像机" << std::endl;
    std::cout << "  +/-      — 增加/减少细分级别" << std::endl;
    std::cout << "  ESC      — 退出" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "  设计说明:" << std::endl;
    std::cout << "  - 人物骨架由粗圆柱/台体（6边形截面）+ SLERP 关节连接器构成" << std::endl;
    std::cout << "  - 顶点着色器负责行走动画（摆臂/摆腿/弹跳）" << std::endl;
    std::cout << "  - 曲面细分阶段使用 PN 三角形使表面光滑" << std::endl;
    std::cout << "  - SLERP 关节连接器在几何阶段生成，替代关节球平滑过渡" << std::endl;
    std::cout << "  - 点光源绕人物轨道运动" << std::endl;
    std::cout << "================================================" << std::endl;
}

// ============================================================
// 主函数
// ============================================================
int main(int argc, char** argv) {
    glutInit(&argc, argv);

#ifdef USE_FREEGLUT
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

    glutInitContextVersion(4, 1);
    glutInitContextProfile(GLUT_CORE_PROFILE);
#ifdef USE_FREEGLUT
    glutInitContextFlags(GLUT_DEBUG);
#endif

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 50);
    int win = glutCreateWindow(WIN_TITLE);

    if (win < 1) {
        std::cerr << "无法创建窗口！" << std::endl;
        return 1;
    }

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW 初始化失败: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    if (!GLEW_ARB_tessellation_shader) {
        std::cerr << "错误：不支持曲面细分着色器 (ARB_tessellation_shader)！" << std::endl;
        return 1;
    }

    std::cout << "OpenGL  " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "渲染器  " << glGetString(GL_RENDERER) << std::endl;
    std::cout << std::endl;

    initGL();
    initCharacter();
    initFloor();
    initPointLightMarker();
    initShaders();

    glutDisplayFunc(displayCallback);
    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialCallback);
    glutReshapeFunc(reshapeCallback);
    glutVisibilityFunc(visibilityCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);
    glutIgnoreKeyRepeat(1);

    printInfo();

    glutMainLoop();

    glDeleteVertexArrays(1, &g.charVAO);
    glDeleteBuffers(1, &g.charVBO);
    glDeleteBuffers(1, &g.charEBO);
    glDeleteVertexArrays(1, &g.floorVAO);
    glDeleteBuffers(1, &g.floorVBO);
    glDeleteBuffers(1, &g.floorEBO);
    glDeleteVertexArrays(1, &g.pointVAO);
    glDeleteBuffers(1, &g.pointVBO);
    glDeleteProgram(g.charProgram);
    glDeleteProgram(g.floorProgram);

    return 0;
}