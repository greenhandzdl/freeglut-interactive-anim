/**
 * @file render/render.h
 * @brief 渲染阶段：OpenGL 资源管理、着色器编译/链接、绘制函数
 */
#pragma once

#include "../common.h"
#include "../geometry/geometry.h"

// ============================================================
// 动画层定义
// ============================================================
enum class AnimLayer {
    IDLE = 0,       // 空闲状态（最低优先级）
    WALK = 1,       // 行走动画
    JUMP = 2,       // 跳跃动画
    WAVE = 3,       // 挥手动画（最高优先级）
    LAYER_COUNT = 4
};

// ============================================================
// 骨骼组定义
// ============================================================
enum class BoneGroup {
    ROOT = 0,       // 根节点（躯干）
    HEAD = 1,       // 头部
    LEFT_ARM = 2,   // 左臂（上臂+前臂）
    RIGHT_ARM = 3,  // 右臂（上臂+前臂）
    LEFT_LEG = 4,   // 左腿（大腿+小腿）
    RIGHT_LEG = 5,  // 右腿（大腿+小腿）
    GROUP_COUNT = 6
};

// ============================================================
// 动画状态
// ============================================================
struct AnimState {
    bool active = false;          // 是否激活
    float weight = 0.0f;          // 权重 (0.0-1.0)
    float phase = 0.0f;           // 动画相位
    float speed = 1.0f;           // 播放速度
};

// ============================================================
// 全局渲染状态
// ============================================================
struct GlobalState {
    // GL 资源
    GLuint charVAO = 0, charVBO = 0, charEBO = 0;
    GLuint floorVAO = 0, floorVBO = 0, floorEBO = 0;
    GLuint pointVAO = 0, pointVBO = 0;
    GLuint charProgram = 0;
    GLuint floorProgram = 0;

    CharacterMesh charMesh;
    FloorMesh     floorMesh;

    // Uniform 位置
    GLint uTime_loc = -1, uJumpPhase_loc = -1;
    GLint uMoveDir_loc = -1, uMoveSpeed_loc = -1;
    GLint uTessLevel_loc = -1;
    GLint uMVP_loc = -1, uModel_loc = -1;
    GLint uLightPos_loc = -1, uLightColor_loc = -1, uLightRadius_loc = -1;
    GLint uAmbient_loc = -1, uColors_loc = -1;
    GLint uCoolColor_loc = -1, uWarmColor_loc = -1;
    GLint uCamPos_loc = -1;
    GLint floorMVP_loc = -1, floorColor_loc = -1;
    
    // 动画层Uniform位置
    GLint uWavePhase_loc = -1;
    GLint uWaveWeight_loc = -1;
    GLint uCameraAngle_loc = -1;  // 相机角度用于挥手方向
    
    // 光照参数
    float lightIntensity = 1.0f;  // 光源亮度强度
    
    // 动画状态 - 按层组织
    AnimState animLayers[(int)AnimLayer::LAYER_COUNT];
    
    // 骨骼组当前激活的层
    int boneGroupActiveLayer[(int)BoneGroup::GROUP_COUNT];

    // 动画状态
    float time = 0.0f;
    float jumpPhase = 0.0f;
    bool  jumping = false;
    float moveSpeed = 0.0f;
    float moveDirX = 0.0f, moveDirZ = -1.0f;
    bool  keys[256] = {false};
    
    // 挥手状态
    bool waving = false;
    float wavePhase = 0.0f;

    // 渲染参数
    float tessLevel = 4.0f;
    float cameraAngle = 0.0f;
    float cameraDist = 5.5f;
    float cameraHeight = 2.5f;
    float modelRotY = 0.0f;

    // 鼠标
    bool mouseDragging = false;
    int mouseLastX = 0, mouseLastY = 0;

    // 人物世界位置
    float charPosX = 0.0f, charPosZ = 0.0f;
};

// ============================================================
// 全局实例
// ============================================================
extern GlobalState g;

// ============================================================
// 着色器工具
// ============================================================
GLuint compileShader(GLenum type, const char* source);
GLuint linkProgram(GLuint vs, GLuint tcs, GLuint tes, GLuint fs);

// ============================================================
// 初始化函数
// ============================================================
void initGL();
void initCharacter();
void initFloor();
void initPointLightMarker();
void initShaders();

// ============================================================
// 每帧更新与渲染
// ============================================================
void updateAnimation(float dt);
Vec3 getLightPos();
void renderScene();