/**
 * @file geometry/geometry.h
 * @brief 几何阶段：网格生成（圆柱、台体、球体、SLERP 关节连接器）
 */
#pragma once

#include <vector>
#include <GL/glew.h>
#include "../math/math.h"

// ============================================================
// 顶点属性索引（与着色器 layout 一致）
// ============================================================
enum AttribIndex {
    ATTR_POSITION  = 0,
    ATTR_NORMAL    = 1,
    ATTR_TEXCOORD  = 2,
    ATTR_BONE_ID   = 3,
};

// ============================================================
// 顶点结构
// ============================================================
struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float tu, tv;
    int   boneId;
};

// ============================================================
// 骨骼 ID 枚举
// ============================================================
enum BoneId {
    BONE_TORSO       = 0,
    BONE_HEAD        = 1,
    BONE_L_UPPER_ARM = 2,
    BONE_L_FOREARM   = 3,
    BONE_R_UPPER_ARM = 4,
    BONE_R_FOREARM   = 5,
    BONE_L_UPPER_LEG = 6,
    BONE_L_LOWER_LEG = 7,
    BONE_R_UPPER_LEG = 8,
    BONE_R_LOWER_LEG = 9,
};

// ============================================================
// 人物网格
// ============================================================
struct CharacterMesh {
    std::vector<Vertex>  vertices;
    std::vector<GLuint>  indices;
    int vertexCount() const { return (int)vertices.size(); }
    int indexCount()  const { return (int)indices.size();  }
};

// ============================================================
// 地面网格
// ============================================================
struct FloorMesh {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

// ============================================================
// SLERP 关节连接器定义
// ============================================================
struct JointConnectorDef {
    float parentEndX, parentEndY, parentEndZ; // 父骨骼末端（父骨骼局部空间）
    float jointX, jointY, jointZ;             // 关节中心（父骨骼局部空间）
    float startRadius, endRadius;             // 起始/结束半径
    int   parentBone;                         // 父骨骼 ID
};

// ============================================================
// 网格生成函数
// ============================================================
void genCylinder(float cx, float cy, float cz,
                 float radius, float height,
                 int sides, int rings,
                 int boneId,
                 std::vector<Vertex>& verts,
                 std::vector<GLuint>& idx);

void genFrustum(float cx, float cy, float cz,
                float topR, float botR, float height,
                int sides, int rings,
                int boneId,
                std::vector<Vertex>& verts,
                std::vector<GLuint>& idx);

void genSphere(float cx, float cy, float cz,
               float radius,
               int lats, int lons,
               int boneId,
               std::vector<Vertex>& verts,
               std::vector<GLuint>& idx);

/**
 * @brief 生成 SLERP 关节连接器
 *
 * 在父骨骼末端与关节中心之间生成平滑过渡网格，
 * 使用 SLERP 对环形位置和法线做球面插值，避免运动时产生空隙。
 * 所有顶点归属于父骨骼。
 */
void genJointConnector(const JointConnectorDef& def,
                       int sides, int rings,
                       std::vector<Vertex>& verts,
                       std::vector<GLuint>& idx);

// ============================================================
// 构建函数
// ============================================================
CharacterMesh buildCharacter();
FloorMesh     buildFloor();