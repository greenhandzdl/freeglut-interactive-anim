/**
 * @file geometry/geometry.cpp
 * @brief 几何阶段实现：网格生成、SLERP 关节连接器、人物构建
 */
#include "geometry.h"
#include "../math/math.h"

#include <iostream>
#include <cmath>
#include <GL/glew.h>

// ============================================================
// 圆柱体
// ============================================================
void genCylinder(float cx, float cy, float cz,
                 float radius, float height,
                 int sides, int rings,
                 int boneId,
                 std::vector<Vertex>& verts,
                 std::vector<GLuint>& idx)
{
    float halfH = height * 0.5f;
    int base = (int)verts.size();

    for (int j = 0; j <= rings; ++j) {
        float v = (float)j / (float)rings;
        float y = cy - halfH + v * height;
        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / (float)sides;
            float theta = u * 2.0f * MATH_PI;
            float x = cx + radius * cosf(theta);
            float z = cz + radius * sinf(theta);
            Vertex vert;
            vert.px = x; vert.py = y; vert.pz = z;
            vert.nx = cosf(theta); vert.ny = 0.0f; vert.nz = sinf(theta);
            vert.tu = u; vert.tv = v;
            vert.boneId = boneId;
            verts.push_back(vert);
        }
    }

    for (int j = 0; j < rings; ++j) {
        for (int i = 0; i < sides; ++i) {
            int a = base + j * (sides + 1) + i;
            int b = a + 1;
            int c = base + (j + 1) * (sides + 1) + i;
            int d = c + 1;
            idx.push_back(a);  idx.push_back(c);  idx.push_back(b);
            idx.push_back(b);  idx.push_back(c);  idx.push_back(d);
        }
    }
}

// ============================================================
// 台体（圆台）
// ============================================================
void genFrustum(float cx, float cy, float cz,
                float topR, float botR, float height,
                int sides, int rings,
                int boneId,
                std::vector<Vertex>& verts,
                std::vector<GLuint>& idx)
{
    float halfH = height * 0.5f;
    int base = (int)verts.size();

    for (int j = 0; j <= rings; ++j) {
        float v = (float)j / (float)rings;
        float y = cy - halfH + v * height;
        float r = topR * (1.0f - v) + botR * v;
        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / (float)sides;
            float theta = u * 2.0f * MATH_PI;
            float x = cx + r * cosf(theta);
            float z = cz + r * sinf(theta);
            Vertex vert;
            vert.px = x; vert.py = y; vert.pz = z;
            vert.nx = cosf(theta); vert.ny = 0.0f; vert.nz = sinf(theta);
            vert.tu = u; vert.tv = v;
            vert.boneId = boneId;
            verts.push_back(vert);
        }
    }

    for (int j = 0; j < rings; ++j) {
        for (int i = 0; i < sides; ++i) {
            int a = base + j * (sides + 1) + i;
            int b = a + 1;
            int c = base + (j + 1) * (sides + 1) + i;
            int d = c + 1;
            idx.push_back(a);  idx.push_back(c);  idx.push_back(b);
            idx.push_back(b);  idx.push_back(c);  idx.push_back(d);
        }
    }
}

// ============================================================
// UV 球体
// ============================================================
void genSphere(float cx, float cy, float cz,
               float radius,
               int lats, int lons,
               int boneId,
               std::vector<Vertex>& verts,
               std::vector<GLuint>& idx)
{
    int base = (int)verts.size();

    for (int j = 0; j <= lats; ++j) {
        float v = (float)j / (float)lats;
        float theta = v * MATH_PI;
        float y = cy + radius * cosf(theta);
        float r = radius * sinf(theta);
        for (int i = 0; i <= lons; ++i) {
            float u = (float)i / (float)lons;
            float phi = u * 2.0f * MATH_PI;
            float x = cx + r * cosf(phi);
            float z = cz + r * sinf(phi);
            Vertex vert;
            vert.px = x; vert.py = y; vert.pz = z;
            float nx = x - cx, ny = y - cy, nz = z - cz;
            float nl = sqrtf(nx*nx + ny*ny + nz*nz);
            if (nl > 0) { nx /= nl; ny /= nl; nz /= nl; }
            vert.nx = nx; vert.ny = ny; vert.nz = nz;
            vert.tu = u; vert.tv = v;
            vert.boneId = boneId;
            verts.push_back(vert);
        }
    }

    for (int j = 0; j < lats; ++j) {
        for (int i = 0; i < lons; ++i) {
            int a = base + j * (lons + 1) + i;
            int b = a + 1;
            int c = base + (j + 1) * (lons + 1) + i;
            int d = c + 1;
            idx.push_back(a);  idx.push_back(c);  idx.push_back(b);
            idx.push_back(b);  idx.push_back(c);  idx.push_back(d);
        }
    }
}

// ============================================================
// SLERP 关节连接器
//
// 在父骨骼末端与关节中心之间生成平滑过渡网格。
// 各环的方向使用 SLERP（球面线性插值）从起始方向过渡到结束方向，
// 使关节区域在骨骼运动时呈现平滑的「波纹管」效果，避免镂空。
// 所有顶点归属于父骨骼。
// ============================================================
void genJointConnector(const JointConnectorDef& def,
                       int sides, int rings,
                       std::vector<Vertex>& verts,
                       std::vector<GLuint>& idx)
{
    int base = (int)verts.size();

    float sx = def.parentEndX, sy = def.parentEndY, sz = def.parentEndZ;
    float ex = def.jointX,    ey = def.jointY,    ez = def.jointZ;
    float startR = def.startRadius;
    float endR   = def.endRadius;
    int boneId   = def.parentBone;

    // 连接器轴向（从父骨骼末端指向关节中心）
    float dx = ex - sx, dy = ey - sy, dz = ez - sz;
    float axisLen = sqrtf(dx*dx + dy*dy + dz*dz);
    if (axisLen < 0.0001f) return; // 退化

    // 起始方向（父骨骼局部 Y 轴）
    float dir0[3] = { 0.0f, 1.0f, 0.0f };
    // 结束方向（连接器指向关节的方向，归一化）
    float dir1[3] = { dx / axisLen, dy / axisLen, dz / axisLen };

    // 计算 SLERP 旋转轴和角度：从 dir0 到 dir1
    float rotAxis[3];
    float cross[3] = {
        dir0[1]*dir1[2] - dir0[2]*dir1[1],
        dir0[2]*dir1[0] - dir0[0]*dir1[2],
        dir0[0]*dir1[1] - dir0[1]*dir1[0]
    };
    float crossLen = sqrtf(cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2]);
    float dot = dir0[0]*dir1[0] + dir0[1]*dir1[1] + dir0[2]*dir1[2];
    dot = fmaxf(-1.0f, fminf(1.0f, dot)); // clamp
    float totalAngle = acosf(dot);

    if (crossLen > 0.0001f) {
        rotAxis[0] = cross[0] / crossLen;
        rotAxis[1] = cross[1] / crossLen;
        rotAxis[2] = cross[2] / crossLen;
    } else {
        // 方向几乎平行或反平行
        rotAxis[0] = 1.0f; rotAxis[1] = 0.0f; rotAxis[2] = 0.0f;
        if (dot < 0.0f) totalAngle = MATH_PI; // 反平行：绕 X 转 180°
    }

    // 为环形生成构建局部基向量
    // 第一环的 tangent = (1,0,0) 在起始方向上的垂直分量
    float tx, ty, tz;
    {
        // 用 (1,0,0) 与 dir0 叉积得到垂直向量
        float cpx = 0.0f, cpy = -dir0[2], cpz = dir0[1]; // (1,0,0) × dir0
        float cl = sqrtf(cpx*cpx + cpy*cpy + cpz*cpz);
        if (cl > 0.0001f) { tx = cpx/cl; ty = cpy/cl; tz = cpz/cl; }
        else { tx = 0.0f; ty = 0.0f; tz = 1.0f; } // dir0 沿 X 时回退
    }

    for (int j = 0; j <= rings; ++j) {
        float t = (float)j / (float)rings; // [0,1]

        // Lerp 环中心位置
        float cx = sx * (1.0f - t) + ex * t;
        float cy = sy * (1.0f - t) + ey * t;
        float cz = sz * (1.0f - t) + ez * t;

        // Lerp 半径
        float r = startR * (1.0f - t) + endR * t;

        // SLERP 环的方向
        float angle = totalAngle * t;
        float ca = cosf(angle), sa = sinf(angle);
        // 绕 rotAxis 旋转 dir0 得到当前环朝向
        float oc = 1.0f - ca;
        float rx = rotAxis[0], ry = rotAxis[1], rz = rotAxis[2];
        float ringDir[3] = {
            dir0[0]*(ca + rx*rx*oc) + dir0[1]*(rx*ry*oc - rz*sa) + dir0[2]*(rx*rz*oc + ry*sa),
            dir0[0]*(ry*rx*oc + rz*sa) + dir0[1]*(ca + ry*ry*oc) + dir0[2]*(ry*rz*oc - rx*sa),
            dir0[0]*(rz*rx*oc - ry*sa) + dir0[1]*(rz*ry*oc + rx*sa) + dir0[2]*(ca + rz*rz*oc)
        };

        // 当前环的 tangent：将起始 tangent 绕同一轴旋转
        float tanRot[3];
        tanRot[0] = tx*(ca + rx*rx*oc) + ty*(rx*ry*oc - rz*sa) + tz*(rx*rz*oc + ry*sa);
        tanRot[1] = tx*(ry*rx*oc + rz*sa) + ty*(ca + ry*ry*oc) + tz*(ry*rz*oc - rx*sa);
        tanRot[2] = tx*(rz*rx*oc - ry*sa) + ty*(rz*ry*oc + rx*sa) + tz*(ca + rz*rz*oc);

        // Bitangent = ringDir × tangent
        float bx = ringDir[1]*tanRot[2] - ringDir[2]*tanRot[1];
        float by = ringDir[2]*tanRot[0] - ringDir[0]*tanRot[2];
        float bz = ringDir[0]*tanRot[1] - ringDir[1]*tanRot[0];

        for (int i = 0; i <= sides; ++i) {
            float u = (float)i / (float)sides;
            float theta = u * 2.0f * MATH_PI;
            float ct = cosf(theta), st = sinf(theta);

            // 环上顶点位置
            float px = cx + (tanRot[0]*ct + bx*st) * r;
            float py = cy + (tanRot[1]*ct + by*st) * r;
            float pz = cz + (tanRot[2]*ct + bz*st) * r;

            // 法线 = 径向方向
            float nx = tanRot[0]*ct + bx*st;
            float ny = tanRot[1]*ct + by*st;
            float nz = tanRot[2]*ct + bz*st;
            float nl = sqrtf(nx*nx + ny*ny + nz*nz);
            if (nl > 0.0001f) { nx /= nl; ny /= nl; nz /= nl; }

            Vertex vert;
            vert.px = px; vert.py = py; vert.pz = pz;
            vert.nx = nx; vert.ny = ny; vert.nz = nz;
            vert.tu = u; vert.tv = t;
            vert.boneId = boneId;
            verts.push_back(vert);
        }
    }

    // 三角形带索引
    for (int j = 0; j < rings; ++j) {
        for (int i = 0; i < sides; ++i) {
            int a = base + j * (sides + 1) + i;
            int b = a + 1;
            int c = base + (j + 1) * (sides + 1) + i;
            int d = c + 1;
            idx.push_back(a);  idx.push_back(c);  idx.push_back(b);
            idx.push_back(b);  idx.push_back(c);  idx.push_back(d);
        }
    }
}

// ============================================================
// 关节连接器定义：替换丑陋的关节球
//
// 每个连接器在父骨骼末端与关节中心之间生成平滑过渡网格，
// 归属于父骨骼，随父骨骼运动而运动，自动覆盖关节空隙。
// ============================================================
static const JointConnectorDef CONNECTORS[] = {
    // ===== 肩关节：从躯干外侧到上臂顶端 =====
    // 左肩：肩关节(-0.26,0.90,0)，上臂半径0.055
    { -0.20f, 0.90f, 0.0f,   -0.26f, 0.90f, 0.0f,   0.055f, 0.055f, BONE_TORSO },
    // 右肩
    {  0.20f, 0.90f, 0.0f,    0.26f, 0.90f, 0.0f,   0.055f, 0.055f, BONE_TORSO },

    // ===== 肘关节：从上臂底部到肘关节中心 =====
    // 左上臂 y 范围 [0.52, 0.90]，肘关节 (-0.28,0.48,0)
    // 连接器从 y=0.52 延伸到 y=0.48
    { -0.26f, 0.52f, 0.0f,   -0.28f, 0.48f, 0.0f,   0.055f, 0.045f, BONE_L_UPPER_ARM },
    // 右上臂
    {  0.26f, 0.52f, 0.0f,    0.28f, 0.48f, 0.0f,   0.055f, 0.045f, BONE_R_UPPER_ARM },

    // ===== 髋关节：从躯干底部到髋关节 =====
    // 躯干底部 y=0.15，髋关节 (-0.10,0.10,0)
    { -0.10f, 0.15f, 0.0f,   -0.10f, 0.10f, 0.0f,   0.085f, 0.12f,  BONE_TORSO },
    {  0.10f, 0.15f, 0.0f,    0.10f, 0.10f, 0.0f,   0.085f, 0.12f,  BONE_TORSO },

    // ===== 膝关节：从大腿底部到膝关节 =====
    // 左大腿 y 范围 [-0.35, 0.10]，膝关节 (-0.11,-0.40,0)
    { -0.10f, -0.35f, 0.0f,  -0.11f, -0.40f, 0.0f,  0.085f, 0.065f,  BONE_L_UPPER_LEG },
    // 右大腿
    {  0.10f, -0.35f, 0.0f,   0.11f, -0.40f, 0.0f,  0.085f, 0.065f,  BONE_R_UPPER_LEG },
};
static const int NUM_CONNECTORS = sizeof(CONNECTORS) / sizeof(CONNECTORS[0]);

// ============================================================
// 构建人物网格
//
// 使用粗圆柱/台体（6 边形截面）+ SLERP 关节连接器，
// 曲面细分阶段使其光滑呈现人形。
// ============================================================
CharacterMesh buildCharacter() {
    CharacterMesh mesh;
    int sides = 6;
    int rings = 2;

    // ---- 躯干 ----
    genFrustum(0.0f, 0.55f, 0.0f, 0.24f, 0.20f, 0.80f,
               sides, rings, BONE_TORSO, mesh.vertices, mesh.indices);

    // ---- 头部 ----
    genSphere(0.0f, 1.05f, 0.0f, 0.16f,
              4, sides, BONE_HEAD, mesh.vertices, mesh.indices);

    // ---- 左臂 ----
    // 左上臂：y [0.52, 0.90]，半径 0.055
    genCylinder(-0.26f, 0.71f, 0.0f, 0.055f, 0.38f,
                6, 2, BONE_L_UPPER_ARM, mesh.vertices, mesh.indices);
    // 左前臂：y [0.10, 0.48]，半径 0.045
    genCylinder(-0.28f, 0.29f, 0.0f, 0.045f, 0.38f,
                6, 2, BONE_L_FOREARM, mesh.vertices, mesh.indices);

    // ---- 右臂 ----
    // 右上臂：y [0.52, 0.90]，半径 0.055
    genCylinder(0.26f, 0.71f, 0.0f, 0.055f, 0.38f,
                6, 2, BONE_R_UPPER_ARM, mesh.vertices, mesh.indices);
    // 右前臂：y [0.10, 0.48]，半径 0.045
    genCylinder(0.28f, 0.29f, 0.0f, 0.045f, 0.38f,
                6, 2, BONE_R_FOREARM, mesh.vertices, mesh.indices);

    // ---- 左腿 ----
    // 左大腿：y [-0.35, 0.10]，半径 0.085
    genCylinder(-0.10f, -0.125f, 0.0f, 0.085f, 0.45f,
                6, 2, BONE_L_UPPER_LEG, mesh.vertices, mesh.indices);
    // 左小腿：y [-0.85, -0.40]，半径 0.065
    genCylinder(-0.11f, -0.625f, 0.0f, 0.065f, 0.45f,
                6, 2, BONE_L_LOWER_LEG, mesh.vertices, mesh.indices);

    // ---- 右腿 ----
    // 右大腿：y [-0.35, 0.10]，半径 0.085
    genCylinder(0.10f, -0.125f, 0.0f, 0.085f, 0.45f,
                6, 2, BONE_R_UPPER_LEG, mesh.vertices, mesh.indices);
    // 右小腿：y [-0.85, -0.40]，半径 0.065
    genCylinder(0.11f, -0.625f, 0.0f, 0.065f, 0.45f,
                6, 2, BONE_R_LOWER_LEG, mesh.vertices, mesh.indices);

    // ---- SLERP 关节连接器（替代丑陋的关节球）----
    for (int j = 0; j < NUM_CONNECTORS; ++j) {
        genJointConnector(CONNECTORS[j], sides, 2,
                          mesh.vertices, mesh.indices);
    }

    std::cout << "[人物] 顶点数: " << mesh.vertexCount()
              << "  三角形数: " << (mesh.indexCount() / 3) << std::endl;
    return mesh;
}

// ============================================================
// 构建地面网格
// ============================================================
FloorMesh buildFloor() {
    FloorMesh floor;
    float size = 12.0f;
    int divs = 20;
    float half = size * 0.5f;

    for (int j = 0; j <= divs; ++j) {
        float v = (float)j / (float)divs;
        float z = -half + v * size;
        for (int i = 0; i <= divs; ++i) {
            float u = (float)i / (float)divs;
            float x = -half + u * size;
            Vertex vert;
            vert.px = x; vert.py = -0.85f; vert.pz = z;
            vert.nx = 0.0f; vert.ny = 1.0f; vert.nz = 0.0f;
            vert.tu = u; vert.tv = v;
            vert.boneId = 0;
            floor.vertices.push_back(vert);
        }
    }

    for (int j = 0; j < divs; ++j) {
        for (int i = 0; i < divs; ++i) {
            int a = j * (divs + 1) + i;
            int b = a + 1;
            int c = (j + 1) * (divs + 1) + i;
            int d = c + 1;
            floor.indices.push_back(a);
            floor.indices.push_back(c);
            floor.indices.push_back(b);
            floor.indices.push_back(b);
            floor.indices.push_back(c);
            floor.indices.push_back(d);
        }
    }

    return floor;
}