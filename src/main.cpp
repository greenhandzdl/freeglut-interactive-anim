/**
 * @file main.cpp
 * @brief FreeGLUT Template Application
 *
 * A basic FreeGLUT template demonstrating cross-platform OpenGL setup.
 * Works on macOS (with XQuartz) and Linux.
 */

#include "common.h"
#include <iostream>
#include <cstdlib>

// Global variables for animation
static float g_rotation = 0.0f;
static bool g_is_rotating = true;
static bool g_first_display = true;
static int g_window_id = -1;

/**
 * @brief Cleanup function
 */
void cleanup() {
    if (g_window_id > 0) {
        glutDestroyWindow(g_window_id);
    }
}

/**
 * @brief Display callback - renders the scene
 */
void displayCallback()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camera setup
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(g_rotation, 0.0f, 1.0f, 0.0f);
    glRotatef(g_rotation * 0.5f, 1.0f, 0.0f, 0.0f);

    // Draw a colorful cube
    glBegin(GL_QUADS);

    // Front face - red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Back face - green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Top face - blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // Bottom face - yellow
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Right face - cyan
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Left face - magenta
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glEnd();

    glutSwapBuffers();

    // Print OpenGL info on first display (when context is valid)
    if (g_first_display)
    {
        g_first_display = false;
        std::cout << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << std::endl;
    }
}

/**
 * @brief Idle callback - updates animation state
 */
void idleCallback()
{
    if (g_is_rotating)
    {
        g_rotation += 1.0f;
        if (g_rotation >= 360.0f)
        {
            g_rotation = 0.0f;
        }
    }

    glutPostRedisplay();
}

/**
 * @brief Keyboard input handler
 */
void keyboardCallback(unsigned char key, int x, int y)
{
    (void)x;
    (void)y;

    switch (key)
    {
        case 27: // ESC
#ifdef USE_FREEGLUT
            glutLeaveMainLoop();
#else
            // macOS native GLUT doesn't have glutLeaveMainLoop
            std::cout << "Exiting..." << std::endl;
            cleanup();
            std::exit(0);
#endif
            break;
        case 'r':
        case 'R':
            g_is_rotating = !g_is_rotating;
            std::cout << "Rotation: " << (g_is_rotating ? "ON" : "OFF") << std::endl;
            break;
        case ' ':
            g_rotation = 0.0f;
            std::cout << "Rotation reset" << std::endl;
            break;
    }
}

/**
 * @brief Window reshape callback
 */
void reshapeCallback(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<GLfloat>(width) / static_cast<GLfloat>(height), 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Initialize OpenGL state
 */
void initGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

/**
 * @brief Print usage information
 */
void printInfo()
{
    std::cout << "=====================================" << std::endl;
    std::cout << " FreeGLUT Template Application" << std::endl;
    std::cout << " Version: " << APP_VERSION << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << " Controls:" << std::endl;
    std::cout << " R     - Toggle rotation" << std::endl;
    std::cout << " SPACE - Reset rotation" << std::endl;
    std::cout << " ESC   - Exit" << std::endl;
    std::cout << "=====================================" << std::endl;
}

/**
 * @brief Main entry point
 */
int main(int argc, char** argv)
{
    // Initialize GLUT first
    glutInit(&argc, argv);

#ifdef USE_FREEGLUT
    // FreeGLUT specific: allow main loop to return
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

    // Print info
    printInfo();

    // Set display mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Set window size and position
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);

    // Create window
    g_window_id = glutCreateWindow(WINDOW_TITLE);

    if (g_window_id < 1) {
        std::cerr << "Error: Failed to create GLUT window" << std::endl;
        return 1;
    }

    std::cout << "Window created successfully (ID: " << g_window_id << ")" << std::endl;

#ifdef USE_GLEW
 // Initialize GLEW (Note: May have issues on macOS with FreeGLUT/X11)
 std::cout << "Attempting GLEW initialization..." << std::endl;
 glewExperimental = GL_TRUE;
 GLenum err = glewInit();
 if (err == GLEW_OK) {
     std::cout << "GLEW initialized successfully!" << std::endl;
     std::cout << "  GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
 } else {
     std::cerr << "Warning: GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
     std::cerr << "Continuing with basic OpenGL..." << std::endl;
 }
#else
 std::cout << "Building without GLEW support" << std::endl;
#endif


    // Initialize OpenGL
    initGL();

    // Register callbacks
    glutDisplayFunc(displayCallback);
    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardCallback);
    glutReshapeFunc(reshapeCallback);

    // Register cleanup
    atexit(cleanup);

    std::cout << "Display: " << (getenv("DISPLAY") ? getenv("DISPLAY") : "not set") << std::endl;
    std::cout << std::endl;

    // Enter main loop
    glutMainLoop();

    std::cout << "Main loop ended" << std::endl;

    return 0;
}
