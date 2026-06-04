/**
 * @file render/render.cpp
 * @brief 渲染阶段实现：GL 初始化、着色器编译/链接、每帧更新与绘制
 */
#include "render.h"
#include "../common.h"
#include "../math/math.h"
#include "../geometry/geometry.h"
#include "../shaders/shaders.h"

#include <iostream>
#include <cmath>
#include <algorithm>


// ============================================================
// 全局状态实例
// ============================================================
GlobalState g;

// ============================================================
// 着色器工具函数
// ============================================================
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        const char* typeName = (type == GL_VERTEX_SHADER) ? "VS" :
                               (type == GL_TESS_CONTROL_SHADER) ? "TCS" :
                               (type == GL_TESS_EVALUATION_SHADER) ? "TES" : "FS";
        std::cerr << "!! " << typeName << " 编译错误:\n" << log << std::endl;
        return 0;
    }
    return shader;
}

GLuint linkProgram(GLuint vs, GLuint tcs, GLuint tes, GLuint fs) {
    GLuint prog = glCreateProgram();
    if (vs)  glAttachShader(prog, vs);
    if (tcs) glAttachShader(prog, tcs);
    if (tes) glAttachShader(prog, tes);
    if (fs)  glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        std::cerr << "!! 链接错误:\n" << log << std::endl;
        return 0;
    }
    return prog;
}

// ============================================================
// GL 状态初始化
// ============================================================
void initGL() {
    glClearColor(0.08f, 0.10f, 0.15f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(3.0f);
}

// ============================================================
// 初始化人物 VAO/VBO/EBO
// ============================================================
void initCharacter() {
    g.charMesh = buildCharacter();

    glGenVertexArrays(1, &g.charVAO);
    glGenBuffers(1, &g.charVBO);
    glGenBuffers(1, &g.charEBO);
    glBindVertexArray(g.charVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g.charVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g.charMesh.vertices.size() * sizeof(Vertex),
                 g.charMesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.charEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 g.charMesh.indices.size() * sizeof(GLuint),
                 g.charMesh.indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, px));
    glEnableVertexAttribArray(ATTR_POSITION);
    glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, nx));
    glEnableVertexAttribArray(ATTR_NORMAL);
    glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, tu));
    glEnableVertexAttribArray(ATTR_TEXCOORD);
    glVertexAttribIPointer(ATTR_BONE_ID, 1, GL_INT,
                           sizeof(Vertex), (void*)offsetof(Vertex, boneId));
    glEnableVertexAttribArray(ATTR_BONE_ID);

    glBindVertexArray(0);
}

// ============================================================
// 初始化点光源标记 VAO
// ============================================================
void initPointLightMarker() {
    glGenVertexArrays(1, &g.pointVAO);
    glGenBuffers(1, &g.pointVBO);
    glBindVertexArray(g.pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g.pointVBO);
    float dummy[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, 5 * sizeof(float), dummy, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// ============================================================
// 初始化地面 VAO/VBO/EBO
// ============================================================
void initFloor() {
    g.floorMesh = buildFloor();

    glGenVertexArrays(1, &g.floorVAO);
    glGenBuffers(1, &g.floorVBO);
    glGenBuffers(1, &g.floorEBO);
    glBindVertexArray(g.floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g.floorVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g.floorMesh.vertices.size() * sizeof(Vertex),
                 g.floorMesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 g.floorMesh.indices.size() * sizeof(GLuint),
                 g.floorMesh.indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, px));
    glEnableVertexAttribArray(ATTR_POSITION);
    glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, tu));
    glEnableVertexAttribArray(ATTR_TEXCOORD);

    glBindVertexArray(0);
}

// ============================================================
// 初始化着色器程序
// ============================================================
void initShaders() {
    GLuint vs  = compileShader(GL_VERTEX_SHADER, VS_SOURCE);
    GLuint tcs = compileShader(GL_TESS_CONTROL_SHADER, TCS_SOURCE);
    GLuint tes = compileShader(GL_TESS_EVALUATION_SHADER, TES_SOURCE);
    GLuint fs  = compileShader(GL_FRAGMENT_SHADER, FS_SOURCE);

    if (!vs || !tcs || !tes || !fs) {
        std::cerr << "人物着色器编译失败！" << std::endl;
        exit(1);
    }

    g.charProgram = linkProgram(vs, tcs, tes, fs);
    if (!g.charProgram) {
        std::cerr << "人物着色器链接失败！" << std::endl;
        exit(1);
    }

    g.uTime_loc      = glGetUniformLocation(g.charProgram, "uTime");
    g.uJumpPhase_loc = glGetUniformLocation(g.charProgram, "uJumpPhase");
    g.uMoveDir_loc   = glGetUniformLocation(g.charProgram, "uMoveDir");
    g.uMoveSpeed_loc = glGetUniformLocation(g.charProgram, "uMoveSpeed");
    g.uTessLevel_loc = glGetUniformLocation(g.charProgram, "uTessLevel");
    g.uMVP_loc       = glGetUniformLocation(g.charProgram, "uMVP");
    g.uModel_loc     = glGetUniformLocation(g.charProgram, "uModel");
    g.uLightPos_loc  = glGetUniformLocation(g.charProgram, "uLightPos");
    g.uLightColor_loc= glGetUniformLocation(g.charProgram, "uLightColor");
    g.uLightRadius_loc= glGetUniformLocation(g.charProgram, "uLightRadius");
    g.uAmbient_loc   = glGetUniformLocation(g.charProgram, "uAmbient");
    g.uColors_loc    = glGetUniformLocation(g.charProgram, "uColors");
    g.uCoolColor_loc = glGetUniformLocation(g.charProgram, "uCoolColor");
    g.uWarmColor_loc = glGetUniformLocation(g.charProgram, "uWarmColor");
    g.uCamPos_loc    = glGetUniformLocation(g.charProgram, "uCamPos");
    g.uWavePhase_loc = glGetUniformLocation(g.charProgram, "uWavePhase");
    g.uWaveWeight_loc= glGetUniformLocation(g.charProgram, "uWaveWeight");
    g.uCameraAngle_loc = glGetUniformLocation(g.charProgram, "uCameraAngle");

    GLuint flVS = compileShader(GL_VERTEX_SHADER, FLOOR_VS);
    GLuint flFS = compileShader(GL_FRAGMENT_SHADER, FLOOR_FS);
    g.floorProgram = linkProgram(flVS, 0, 0, flFS);
    g.floorMVP_loc  = glGetUniformLocation(g.floorProgram, "uMVP");
    g.floorColor_loc= glGetUniformLocation(g.floorProgram, "uColor");

    glDeleteShader(vs); glDeleteShader(tcs); glDeleteShader(tes); glDeleteShader(fs);
    glDeleteShader(flVS); glDeleteShader(flFS);

    std::cout << "[着色器] 人物程序 ID=" << g.charProgram
              << "  地面程序 ID=" << g.floorProgram << std::endl;
}

// ============================================================
// 每帧更新（动画 + 输入处理）
// ============================================================
void updateAnimation(float dt) {
    // 更新时间 - 始终基于移动速度更新，确保动画连续播放
    // 即使速度很小也要更新时间，避免动画停滞
    float animSpeed = 0.2f + g.moveSpeed * 0.8f;  // 基础速度0.2，最大1.0
    g.time += dt * animSpeed;

    // 跳跃状态机
    if (g.jumping && (g.jumpPhase <= 0.0f || g.jumpPhase >= 1.0f)) {
        g.jumpPhase = 0.001f;
    }

    if (g.jumpPhase > 0.0f && g.jumpPhase < 1.0f) {
        g.jumpPhase += dt * 1.8f;
        if (g.jumpPhase >= 1.0f) {
            g.jumpPhase = 1.0f;
        }
    }

    // 挥手状态机
    if (g.waving) {
        g.wavePhase += dt * 3.0f;  // 加快挥手频率
    } else {
        // 挥手淡出
        if (g.wavePhase > 0.0f) {
            g.wavePhase = 0.0f;
        }
    }

    // 移动加速和摩擦
    float moveAccel = 2.5f * dt;
    float moveFriction = 1.2f * dt;

    // WASD输入处理 - 修复移动方向
    float dx = 0.0f, dz = 0.0f;
    if (g.keys['w'] || g.keys['W']) dz -= 1.0f;
    if (g.keys['s'] || g.keys['S']) dz += 1.0f;
    if (g.keys['a'] || g.keys['A']) dx -= 1.0f;
    if (g.keys['d'] || g.keys['D']) dx += 1.0f;

    float len = sqrtf(dx*dx + dz*dz);
    if (len > 0.001f) {
        dx /= len;
        dz /= len;

        // 修复：将相机坐标系下的输入转换为世界坐标系
        // 相机位置: eye = charPos + (sin(angle)*dist, height, cos(angle)*dist)
        // 当angle=0时，相机在+Z方向（人物后方），看向人物
        // 相机的"前"方向是朝向人物，即(-sin(angle), 0, -cos(angle))
        // 相机的"右"方向是(cos(angle), 0, -sin(angle))
        
        float ca = g.cameraAngle;
        float cosA = cosf(ca);
        float sinA = sinf(ca);
        
        // 相机空间到世界空间的转换
        // W键（相机前）: 世界方向 = (-sinA, 0, -cosA)
        // S键（相机后）: 世界方向 = (sinA, 0, cosA)
        // A键（相机左）: 世界方向 = (-cosA, 0, sinA)
        // D键（相机右）: 世界方向 = (cosA, 0, -sinA)
        
        // 组合移动方向
        float worldDx = dz * sinA + dx * cosA;
        float worldDz = dz * cosA - dx * sinA;
        
        g.moveDirX = worldDx;
        g.moveDirZ = worldDz;

        g.moveSpeed += moveAccel;
        if (g.moveSpeed > 1.2f) g.moveSpeed = 1.2f;
        
        // 人物朝向移动方向
        g.modelRotY = atan2f(worldDx, -worldDz);
    } else {
        g.moveSpeed -= moveFriction;
        if (g.moveSpeed < 0.0f) g.moveSpeed = 0.0f;

        // 空闲时人物朝向相机前方（左键拖拽摄像机时更新朝向）
        g.modelRotY = g.cameraAngle;
    }

    // 应用移动
    if (g.moveSpeed > 0.01f) {
        g.charPosX += g.moveDirX * g.moveSpeed * dt * 0.8f;
        g.charPosZ += g.moveDirZ * g.moveSpeed * dt * 0.8f;
    }
}

// ============================================================
// 点光源位置
// ============================================================
Vec3 getLightPos() {
    float angle = g.time * 0.4f;
    return Vec3(g.charPosX + 4.0f * cosf(angle),
                3.0f,
                g.charPosZ + 4.0f * sinf(angle));
}

// ============================================================
// 渲染场景
// ============================================================
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Vec3 eye(g.charPosX + g.cameraDist * sinf(g.cameraAngle),
             g.cameraHeight,
             g.charPosZ + g.cameraDist * cosf(g.cameraAngle));
    Vec3 center(g.charPosX, 0.0f, g.charPosZ);
    Vec3 up(0, 1, 0);

    float aspect = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
    Mat4 proj = Mat4::perspective(45.0f, aspect, 0.1f, 30.0f);
    Mat4 view = Mat4::lookAt(eye, center, up);

    // ---- 人物渲染 ----
    glUseProgram(g.charProgram);

    Mat4 model = Mat4::translate(g.charPosX, 0.0f, g.charPosZ)
               * Mat4::rotateY(g.modelRotY);
    Mat4 mvp = proj * view * model;

    glUniformMatrix4fv(g.uMVP_loc, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(g.uModel_loc, 1, GL_FALSE, model.m);

    glUniform1f(g.uTime_loc, g.time);
    glUniform1f(g.uJumpPhase_loc, g.jumpPhase);
    glUniform2f(g.uMoveDir_loc, g.moveDirX, g.moveDirZ);
    glUniform1f(g.uMoveSpeed_loc, g.moveSpeed);
    glUniform1f(g.uTessLevel_loc, g.tessLevel);
    
    // 挥手参数
    glUniform1f(g.uWavePhase_loc, g.wavePhase);
    float waveWeight = g.waving ? 1.0f : 0.0f;
    glUniform1f(g.uWaveWeight_loc, waveWeight);
    
    // 相机角度（用于挥手方向）
    glUniform1f(g.uCameraAngle_loc, g.cameraAngle);

    Vec3 lightPos = getLightPos();
    glUniform3f(g.uLightPos_loc, lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(g.uLightColor_loc, 0.9f * g.lightIntensity, 0.85f * g.lightIntensity, 0.8f * g.lightIntensity);
    glUniform1f(g.uLightRadius_loc, 6.0f);
    glUniform3f(g.uAmbient_loc, 0.12f * g.lightIntensity, 0.12f * g.lightIntensity, 0.15f * g.lightIntensity);

    glUniform3f(g.uCoolColor_loc, 0.2f, 0.3f, 0.6f);
    glUniform3f(g.uWarmColor_loc, 0.7f, 0.5f, 0.2f);
    glUniform3f(g.uCamPos_loc, eye.x, eye.y, eye.z);

    float colors[10][3] = {
        {0.25f, 0.50f, 1.00f},
        {1.00f, 0.85f, 0.70f},
        {0.95f, 0.80f, 0.70f},
        {0.95f, 0.80f, 0.70f},
        {0.95f, 0.80f, 0.70f},
        {0.95f, 0.80f, 0.70f},
        {0.20f, 0.25f, 0.60f},
        {0.20f, 0.25f, 0.60f},
        {0.20f, 0.25f, 0.60f},
        {0.20f, 0.25f, 0.60f},
    };
    glUniform3fv(g.uColors_loc, 10, (float*)colors);

    glPatchParameteri(GL_PATCH_VERTICES, 3);

    glBindVertexArray(g.charVAO);
    glDrawElements(GL_PATCHES, (GLsizei)g.charMesh.indices.size(),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // ---- 地面渲染 ----
    glUseProgram(g.floorProgram);
    Mat4 floorModel;
    Mat4 floorMVP = proj * view * floorModel;
    glUniformMatrix4fv(g.floorMVP_loc, 1, GL_FALSE, floorMVP.m);
    glUniform3f(g.floorColor_loc, 0.4f, 0.55f, 0.4f);

    glBindVertexArray(g.floorVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)g.floorMesh.indices.size(),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);

    // ---- 点光源指示小球 ----
    glUseProgram(g.floorProgram);
    glBindVertexArray(g.pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g.pointVBO);
    float ptData[5] = { lightPos.x, lightPos.y, lightPos.z, 0.5f, 0.5f };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ptData), ptData);
    Mat4 lightModel;
    Mat4 lightMVP = proj * view * lightModel;
    glUniformMatrix4fv(g.floorMVP_loc, 1, GL_FALSE, lightMVP.m);
    glUniform3f(g.floorColor_loc, 1.0f, 0.8f, 0.3f);
    glPointSize(14.0f);
    glDrawArrays(GL_POINTS, 0, 1);
    glPointSize(3.0f);
    glBindVertexArray(0);

    glutSwapBuffers();
}