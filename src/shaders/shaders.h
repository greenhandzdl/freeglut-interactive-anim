/**
 * @file shaders/shaders.h
 * @brief 着色器阶段：所有 GLSL 着色器源码字符串
 *
 * 按渲染管线阶段组织：
 *   顶点着色器 → 细分控制(TCS) → 细分求值(TES) → 片段着色器(FS)
 *   + 地面专用顶点/片段着色器
 */
#pragma once

// ---- 顶点着色器：人物动画（骨骼行走/跳跃/防穿插）----
extern const char* VS_SOURCE;

// ---- 曲面细分控制着色器（PN 三角形 + 细分级别）----
extern const char* TCS_SOURCE;

// ---- 曲面细分求值着色器（贝塞尔三角形求值）----
extern const char* TES_SOURCE;

// ---- 片段着色器（Gooch + 玻璃质感 + 骨骼颜色 + 点光源）----
extern const char* FS_SOURCE;

// ---- 地面着色器 ----
extern const char* FLOOR_VS;
extern const char* FLOOR_FS;