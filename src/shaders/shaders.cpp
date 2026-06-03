/**
 * @file shaders/shaders.cpp
 * @brief 着色器源码实现
 */
#include "shaders.h"

// ---- 顶点着色器：人物动画 ----
const char* VS_SOURCE = R"glsl(
#version 400 core
#define PI 3.14159265358979323846
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int  inBoneId;

out vec3 vPos;
out vec3 vNorm;
out vec2 vTex;
flat out int vBone;

uniform float uTime;
uniform float uJumpPhase;
uniform vec2  uMoveDir;
uniform float uMoveSpeed;

vec3 jointPos(int bone) {
    switch (bone) {
        case 0: return vec3(0.00, 0.00, 0.0);
        case 1: return vec3(0.00, 1.05, 0.0);
        case 2: return vec3(-0.26, 0.90, 0.0);
        case 3: return vec3(-0.28, 0.48, 0.0);
        case 4: return vec3( 0.26, 0.90, 0.0);
        case 5: return vec3( 0.28, 0.48, 0.0);
        case 6: return vec3(-0.10, 0.10, 0.0);
        case 7: return vec3(-0.11,-0.40, 0.0);
        case 8: return vec3( 0.10, 0.10, 0.0);
        case 9: return vec3( 0.11,-0.40, 0.0);
        default: return vec3(0.0);
    }
}

vec3 rotateX(vec3 p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec3(p.x, p.y * c - p.z * s, p.y * s + p.z * c);
}

vec3 antiClip(vec3 pos, int bone) {
    if (bone >= 2 && bone <= 5) {
        float torsoBot = 0.10;
        float torsoTop = 0.95;
        float torsoRad = 0.23;
        float dist = length(pos.xz);
        if (dist < torsoRad + 0.01) {
            if (dist < 0.001) {
                if (bone == 2 || bone == 3) pos.xz = vec2(-torsoRad - 0.01, 0.0);
                else pos.xz = vec2(torsoRad + 0.01, 0.0);
            } else {
                pos.xz = normalize(pos.xz) * (torsoRad + 0.01);
            }
        }
    }
    if (bone == 6 || bone == 7) {
        if (pos.x > -0.04) pos.x = -0.04;
    }
    if (bone == 8 || bone == 9) {
        if (pos.x <  0.04) pos.x =  0.04;
    }
    return pos;
}

void main() {
    vec3 pos = inPos;
    vec3 norm = inNormal;
    int bone = inBoneId;

    float walkFreq = 3.0;
    float t = uTime * walkFreq;
    float speedFactor = clamp(uMoveSpeed, 0.0, 1.0);
    float swingAmp = 1.0 * speedFactor;
    float kneeBend = 0.7 * speedFactor;
    float elbowBend = 0.4 * speedFactor;
    float bodyBob = 0.10 * speedFactor;
    float bodySway = 0.05 * speedFactor;
    float hipTwist = 0.12 * speedFactor;

    float jumpH = 0.0;
    float jumpTuck = 0.0;
    float jumpTwist = 0.0;
    if (uJumpPhase > 0.0 && uJumpPhase < 1.0) {
        float phase = uJumpPhase * PI;
        jumpH = 1.0 * sin(phase);
        jumpTuck = 0.6 * sin(phase);
        jumpTwist = 0.3 * sin(phase * 2.0);
    }

    if (bone == 0) {
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        pos.y += bob + jumpH;
        pos.x += sway;
    }
    else if (bone == 1) {
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        float nod = 0.03 * speedFactor * sin(t);
        pos.y += bob + jumpH;
        pos.x += sway;
        vec3 j = jointPos(1);
        pos -= j;
        pos = rotateX(pos, nod);
        pos += j;
    }
    else if (bone == 2 || bone == 4) {
        float sign = (bone == 2) ? 1.0 : -1.0;
        float angle = swingAmp * sign * sin(t);
        float tuck = -jumpTuck * 0.3 * sign;
        angle += tuck;
        vec3 j = jointPos(bone);
        pos -= j;
        pos = rotateX(pos, angle);
        norm = rotateX(norm, angle);
        pos += j;
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        pos.y += bob + jumpH;
        pos.x += sway;
    }
    else if (bone == 3 || bone == 5) {
        float sign = (bone == 3) ? 1.0 : -1.0;
        float shoulderAngle = swingAmp * sign * sin(t);
        float elbowAngle = elbowBend * sign * abs(sin(t));
        float tuck = -jumpTuck * 0.4 * sign;
        elbowAngle += tuck;
        vec3 shoulder = jointPos(bone - 1);
        vec3 elbow    = jointPos(bone);
        vec3 rel = pos - shoulder;
        rel = rotateX(rel, shoulderAngle);
        vec3 foreRel = rel - (elbow - shoulder);
        foreRel = rotateX(foreRel, elbowAngle);
        pos = shoulder + (elbow - shoulder) + foreRel;
        norm = rotateX(norm, shoulderAngle);
        norm = rotateX(norm, elbowAngle);
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        pos.y += bob + jumpH;
        pos.x += sway;
    }
    else if (bone == 6 || bone == 8) {
        float sign = (bone == 6) ? 1.0 : -1.0;
        float angle = swingAmp * sign * sin(t);
        float tuck = -jumpTuck * 0.4 * sign;
        angle += tuck;
        vec3 j = jointPos(bone);
        pos -= j;
        pos = rotateX(pos, angle);
        norm = rotateX(norm, angle);
        pos += j;
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        pos.y += bob + jumpH;
        pos.x += sway;
    }
    else if (bone == 7 || bone == 9) {
        float sign = (bone == 7) ? 1.0 : -1.0;
        float hipAngle = swingAmp * sign * sin(t);
        float kneeAngle = kneeBend * sign * (sin(t) * 0.5 + 0.5);
        float tuck = -jumpTuck * 0.5 * sign;
        kneeAngle += tuck;
        vec3 hip  = jointPos(bone - 1);
        vec3 knee = jointPos(bone);
        vec3 rel = pos - hip;
        rel = rotateX(rel, hipAngle);
        vec3 legRel = rel - (knee - hip);
        legRel = rotateX(legRel, kneeAngle);
        pos = hip + (knee - hip) + legRel;
        norm = rotateX(norm, hipAngle);
        norm = rotateX(norm, kneeAngle);
        float bob = bodyBob * sin(t);
        float sway = bodySway * cos(t);
        pos.y += bob + jumpH;
        pos.x += sway;
    }

    if (jumpTwist != 0.0) {
        float c = cos(jumpTwist);
        float s = sin(jumpTwist);
        pos = vec3(pos.x * c - pos.z * s, pos.y, pos.x * s + pos.z * c);
        norm = vec3(norm.x * c - norm.z * s, norm.y, norm.x * s + norm.z * c);
    }

    pos = antiClip(pos, bone);

    vPos   = pos;
    vNorm  = normalize(norm);
    vTex   = inTexCoord;
    vBone  = bone;
}
)glsl";

// ---- 曲面细分控制着色器（TCS）----
const char* TCS_SOURCE = R"glsl(
#version 400 core
layout(vertices = 3) out;

in  vec3 vPos[];
in  vec3 vNorm[];
in  vec2 vTex[];
flat in int  vBone[];

out vec3 tcPos[];
out vec3 tcNorm[];
out vec2 tcTex[];
flat out int tcBone[];

patch out vec3 p300;
patch out vec3 p030;
patch out vec3 p003;
patch out vec3 p210;
patch out vec3 p120;
patch out vec3 p021;
patch out vec3 p012;
patch out vec3 p102;
patch out vec3 p201;
patch out vec3 p111;

uniform float uTessLevel;

void main() {
    tcPos[gl_InvocationID]  = vPos[gl_InvocationID];
    tcNorm[gl_InvocationID] = vNorm[gl_InvocationID];
    tcTex[gl_InvocationID]  = vTex[gl_InvocationID];
    tcBone[gl_InvocationID] = vBone[gl_InvocationID];

    if (gl_InvocationID == 0) {
        vec3 p1 = vPos[0];
        vec3 p2 = vPos[1];
        vec3 p3 = vPos[2];
        vec3 n1 = vNorm[0];
        vec3 n2 = vNorm[1];
        vec3 n3 = vNorm[2];

        p300 = p1;
        p030 = p2;
        p003 = p3;

        float w12 = dot(p2 - p1, n1);
        float w21 = dot(p1 - p2, n2);
        float w23 = dot(p3 - p2, n2);
        float w32 = dot(p2 - p3, n3);
        float w31 = dot(p1 - p3, n3);
        float w13 = dot(p3 - p1, n1);

        p210 = (2.0 * p1 + p2 - w12 * n1) / 3.0;
        p120 = (2.0 * p2 + p1 - w21 * n2) / 3.0;
        p021 = (2.0 * p2 + p3 - w23 * n2) / 3.0;
        p012 = (2.0 * p3 + p2 - w32 * n3) / 3.0;
        p102 = (2.0 * p3 + p1 - w31 * n3) / 3.0;
        p201 = (2.0 * p1 + p3 - w13 * n1) / 3.0;

        vec3 E = (p210 + p120 + p021 + p012 + p102 + p201) / 6.0;
        vec3 O = (p1 + p2 + p3) / 3.0;
        p111 = E + (E - O) / 2.0;

        float tess = clamp(uTessLevel, 1.0, 12.0);
        gl_TessLevelInner[0] = tess;
        gl_TessLevelOuter[0] = tess;
        gl_TessLevelOuter[1] = tess;
        gl_TessLevelOuter[2] = tess;
    }
}
)glsl";

// ---- 曲面细分求值着色器（TES）----
const char* TES_SOURCE = R"glsl(
#version 400 core
layout(triangles, fractional_even_spacing, ccw) in;

in  vec3 tcPos[];
in  vec3 tcNorm[];
in  vec2 tcTex[];
flat in int tcBone[];

patch in vec3 p300;
patch in vec3 p030;
patch in vec3 p003;
patch in vec3 p210;
patch in vec3 p120;
patch in vec3 p021;
patch in vec3 p012;
patch in vec3 p102;
patch in vec3 p201;
patch in vec3 p111;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTex;
flat out int fBone;

uniform mat4 uMVP;
uniform mat4 uModel;

vec3 bezierPoint(float u, float v, float w) {
    float u2 = u * u; float v2 = v * v; float w2 = w * w;
    float u3 = u2 * u; float v3 = v2 * v; float w3 = w2 * w;
    return p300 * u3
         + p030 * v3
         + p003 * w3
         + 3.0 * u2 * v * p210
         + 3.0 * u * v2 * p120
         + 3.0 * v2 * w * p021
         + 3.0 * v * w2 * p012
         + 3.0 * w2 * u * p102
         + 3.0 * w * u2 * p201
         + 6.0 * u * v * w * p111;
}

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec3 pos = bezierPoint(u, v, w);
    vec3 norm = normalize(tcNorm[0] * u + tcNorm[1] * v + tcNorm[2] * w);
    vec2 tex = tcTex[0] * u + tcTex[1] * v + tcTex[2] * w;

    fPos  = (uModel * vec4(pos, 1.0)).xyz;
    fNorm = normalize((uModel * vec4(norm, 0.0)).xyz);
    fTex  = tex;
    fBone = tcBone[0];

    gl_Position = uMVP * vec4(pos, 1.0);
}
)glsl";

// ---- 片段着色器：Gooch 光照 + 玻璃质感 + 骨骼肤色叠加 + 点光源 ----
const char* FS_SOURCE = R"glsl(
#version 400 core
in vec3 fPos;
in vec3 fNorm;
in vec2 fTex;
flat in int fBone;

out vec4 fragColor;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightRadius;
uniform vec3 uAmbient;

uniform vec3 uCoolColor;
uniform vec3 uWarmColor;
uniform vec3 uCamPos;
uniform vec3 uColors[10];

void main() {
    vec3 N = normalize(fNorm);
    vec3 L = uLightPos - fPos;
    float dist = length(L);
    L /= dist;
    vec3 V = normalize(uCamPos - fPos);
    vec3 H = normalize(L + V);

    float atten = 1.0 / (1.0 + (dist * dist) / (uLightRadius * uLightRadius));
    atten = clamp(atten, 0.0, 1.0);

    float t = dot(N, L) * 0.5 + 0.5;
    vec3 goochColor = mix(uCoolColor, uWarmColor, t);

    float spec = pow(max(dot(N, H), 0.0), 64.0);
    vec3 specColor = uLightColor * spec * 0.8 * atten;

    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);
    vec3 fresnelColor = mix(vec3(0.04), uLightColor, fresnel * 0.5);

    float rim = 1.0 - max(dot(N, V), 0.0);
    vec3 rimColor = uLightColor * pow(rim, 2.0) * 0.3 * atten;

    int idx = clamp(fBone, 0, 9);
    vec3 boneColor = uColors[idx];
    goochColor = mix(goochColor, boneColor, 0.2);

    vec3 finalColor = goochColor * (uAmbient + 0.6 * atten)
                    + specColor
                    + fresnelColor * 0.5
                    + rimColor;

    float alpha = 0.75 + 0.25 * (1.0 - abs(dot(N, V)));

    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    fragColor = vec4(finalColor, alpha);
}
)glsl";

// ---- 地面着色器 ----
const char* FLOOR_VS = R"glsl(
#version 400 core
layout(location = 0) in vec3 inPos;
layout(location = 2) in vec2 inUV;
out vec2 vUV;
uniform mat4 uMVP;
void main() {
    vUV = inUV;
    gl_Position = uMVP * vec4(inPos, 1.0);
}
)glsl";

const char* FLOOR_FS = R"glsl(
#version 400 core
in vec2 vUV;
out vec4 fragColor;
uniform vec3 uColor;
void main() {
    float c = step(0.5, mod(floor(vUV.x * 20.0) + floor(vUV.y * 20.0), 2.0));
    vec3 col = mix(uColor * 0.5, uColor, c);
    float fog = 1.0 - vUV.y * 0.3;
    col *= fog;
    fragColor = vec4(col, 1.0);
}
)glsl";