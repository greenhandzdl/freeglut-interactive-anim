# 模块流程图与职责说明

---

## 一、整体模块依赖关系

```mermaid
graph TB
    subgraph APP["应用层"]
        MAIN["main.cpp<br/>入口 + 回调"]
    end

    subgraph MATH["数学工具层"]
        MATH_H["math.h<br/>Vec3 / Mat4"]
    end

    subgraph GEOMETRY["几何生成层"]
        GEOM_H["geometry.h<br/>类型定义"]
        GEOM_CPP["geometry.cpp<br/>网格生成器"]
    end

    subgraph SHADERS["着色器源码层"]
        SHAD_H["shaders.h<br/>声明"]
        SHAD_CPP["shaders.cpp<br/>GLSL 源码"]
    end

    subgraph RENDER["渲染层"]
        RENDER_H["render.h<br/>全局状态 + 声明"]
        RENDER_CPP["render.cpp<br/>初始化 + 动画 + 绘制"]
    end

    MAIN -->|"调用"| RENDER
    RENDER -->|"包含"| MATH_H
    RENDER -->|"包含"| GEOM_H
    RENDER -->|"包含"| SHAD_H
    RENDER_CPP -->|"链接"| SHAD_CPP
    GEOM_CPP -->|"包含"| GEOM_H
    GEOM_CPP -->|"包含"| MATH_H
```

---

## 二、主流程调用链

```mermaid
flowchart TD
    MAIN["main.cpp<br/>main()"]

    subgraph INIT["初始化阶段 (一次)"]
        INIT_GL["initGL()"]
        INIT_CHAR["initCharacter()"]
        INIT_FLOOR["initFloor()"]
        INIT_LIGHT["initPointLightMarker()"]
        INIT_SHADERS["initShaders()"]
        REG_CB["注册回调"]
    end

    subgraph FRAME["运行阶段 (每帧)"]
        IDLE["idleCallback"]
        UPDATE["updateAnimation(dt)"]
        DISPLAY["displayCallback"]
        RENDER["renderScene()"]
    end

    subgraph INPUT["输入回调"]
        KEYDOWN["keyboardCallback"]
        KEYUP["keyboardUpCallback"]
        SPECIAL["specialCallback"]
        MOUSE_BTN["mouseCallback"]
        MOUSE_MOVE["mouseMotionCallback"]
    end

    MAIN --> INIT_GL
    MAIN --> INIT_CHAR
    MAIN --> INIT_FLOOR
    MAIN --> INIT_LIGHT
    MAIN --> INIT_SHADERS
    MAIN --> REG_CB
    REG_CB -->|"glutIdleFunc"| IDLE
    REG_CB -->|"glutDisplayFunc"| DISPLAY
    REG_CB -->|"glutKeyboardFunc/Up/Mouse/Motion/Special"| INPUT
    IDLE -->|"计算 dt"| UPDATE
    UPDATE -->|"glutPostRedisplay"| DISPLAY
    DISPLAY --> RENDER
```

---

## 三、模块详细说明

### 3.1 应用入口 — `src/main.cpp`

| 项目 | 内容 |
|------|------|
| **作用** | 窗口创建、OpenGL 上下文初始化、FreeGLUT 回调注册、主循环启动 |
| **关键函数** | `main()` |
| **职责** | 1. `glutInit` 设置 OpenGL 4.1 Core Profile<br/>2. `glewInit` 初始化扩展，检查曲面细分支持<br/>3. 按序调用 5 个初始化函数<br/>4. 注册 8 个 FreeGLUT 回调<br/>5. 启动 `glutMainLoop`<br/>6. 退出时清理 GL 资源 |

### 3.2 公共宏 — `src/common.h`

| 项目 | 内容 |
|------|------|
| **作用** | 跨平台 GLUT/GLEW 头文件包含、窗口默认值定义 |
| **关键定义** | 平台宏 `__APPLE__` / `USE_GLEW` / `USE_FREEGLUT` |
| **职责** | 1. macOS 与 Linux 的 GLUT/GLEW 引入顺序处理<br/>2. 窗口宽高默认值、应用名、版本号 |

---

### 3.3 数学工具 — `src/math/math.h`

| 项目 | 内容 |
|------|------|
| **作用** | 手写 3D 数学库，提供 Vec3 和 Mat4 |
| **关键类型** | `struct Vec3`, `struct Mat4` |
| **关键方法** | 见下表 |

```mermaid
graph LR
    subgraph MATH_MOD["math.h"]
        VEC3["Vec3<br/>x, y, z"]
        MAT4["Mat4<br/>float m[16] 列主序"]
        IDEN["setIdentity()"]
        PERSP["perspective(fovY, aspect, near, far)"]
        LOOK["lookAt(eye, center, up)"]
        TRANS["translate(tx, ty, tz)"]
        ROTY["rotateY(angle)"]
        MUL["operator* (矩阵乘法)"]
    end

    subgraph USAGE["使用方"]
        GEOM["geometry.cpp<br/>位置/法线计算"]
        RENDER["render.cpp<br/>MVP 矩阵构建"]
    end

    VEC3 --> GEOM
    PERSP & LOOK & TRANS & ROTY & MUL --> RENDER
```

---

### 3.4 几何生成 — `src/geometry/`

```mermaid
graph TD
    subgraph GEOM_LAYER["geometry 模块 (2 文件)"]
        GEOM_H["geometry.h<br/>类型/枚举/声明"]
        GEOM_CPP["geometry.cpp<br/>生成器实现"]
    end

    GEOM_H ---|定义| VERTEX["Vertex 结构<br/>位置+法线+UV+骨骼ID"]
    GEOM_H ---|定义| BONE_ENUM["BoneId 枚举<br/>10 块骨骼"]
    GEOM_H ---|定义| MESHES["CharacterMesh / FloorMesh"]
    GEOM_H ---|定义| CONN_DEF["JointConnectorDef<br/>连接器参数"]

    GEOM_CPP ---|实现| GEN_CYL["genCylinder()<br/>圆柱体"]
    GEOM_CPP ---|实现| GEN_FRUS["genFrustum()<br/>台体(圆台)"]
    GEOM_CPP ---|实现| GEN_SPH["genSphere()<br/>经纬球体"]
    GEOM_CPP ---|实现| GEN_CONN["genJointConnector()<br/>SLERP 关节连接器"]
    GEOM_CPP ---|实现| BUILD_CHAR["buildCharacter()<br/>组装人物"]
    GEOM_CPP ---|实现| BUILD_FLOOR["buildFloor()<br/>地面网格"]
```

| 文件 | 函数 | 作用 |
|------|------|------|
| `geometry.h` | — | 定义 `Vertex`、`BoneId` 枚举（10 骨）、`CharacterMesh`、`JointConnectorDef`、函数声明 |
| `geometry.cpp` | `genCylinder()` | 生成固定半径圆柱体网格（6 边 × 2 环），法线水平径向 |
| | `genFrustum()` | 生成半径线性变化的台体（圆台） |
| | `genSphere()` | 生成经纬球体，法线精确球面径向 |
| | `genJointConnector()` | SLERP 球面插值生成关节过渡网格 |
| | `buildCharacter()` | 按躯干→头→四肢→关节顺序组装完整人物 |
| | `buildFloor()` | 生成 20×20 方格地面 |

#### 关节连接器定义表 (`geometry.cpp`)

| 连接器 | 起点 (父骨骼末端) | 终点 (关节中心) | 半径 | 归属 |
|--------|------------------|----------------|------|------|
| 左肩 | `(-0.20, 0.90)` | `(-0.26, 0.90)` | 0.055→0.055 | TORSO(0) |
| 右肩 | `( 0.20, 0.90)` | `( 0.26, 0.90)` | 0.055→0.055 | TORSO(0) |
| 左肘 | `(-0.26, 0.52)` | `(-0.28, 0.48)` | 0.055→0.045 | L_UPPER_ARM(2) |
| 右肘 | `( 0.26, 0.52)` | `( 0.28, 0.48)` | 0.055→0.045 | R_UPPER_ARM(4) |
| 左髋 | `(-0.10, 0.15)` | `(-0.10, 0.10)` | 0.085→0.12 | TORSO(0) |
| 右髋 | `( 0.10, 0.15)` | `( 0.10, 0.10)` | 0.085→0.12 | TORSO(0) |
| 左膝 | `(-0.10,-0.35)` | `(-0.11,-0.40)` | 0.085→0.065 | L_UPPER_LEG(6) |
| 右膝 | `( 0.10,-0.35)` | `( 0.11,-0.40)` | 0.085→0.065 | R_UPPER_LEG(8) |

---

### 3.5 着色器源码 — `src/shaders/`

```mermaid
graph LR
    subgraph SHADERS["shaders 模块 (2 文件)"]
        SHAD_H["shaders.h<br/>6 个 extern const char* 声明"]
        SHAD_CPP["shaders.cpp<br/>6 段 GLSL 源码"]
    end

    subgraph PROGRAMS["2 个着色器程序"]
        CHAR_PROG["charProgram<br/>VS + TCS + TES + FS"]
        FLOOR_PROG["floorProgram<br/>VS + FS"]
    end

    subgraph CHAR_PIPELINE["人物管线 (4 阶段)"]
        VS["VS_SOURCE<br/>顶点着色器"]
        TCS["TCS_SOURCE<br/>细分控制"]
        TES["TES_SOURCE<br/>细分求值"]
        FS["FS_SOURCE<br/>片段着色器"]
    end

    subgraph FLOOR_PIPELINE["地面管线 (2 阶段)"]
        FLOOR_VS["FLOOR_VS"]
        FLOOR_FS["FLOOR_FS"]
    end

    SHAD_CPP --> VS
    SHAD_CPP --> TCS
    SHAD_CPP --> TES
    SHAD_CPP --> FS
    SHAD_CPP --> FLOOR_VS
    SHAD_CPP --> FLOOR_FS

    VS & TCS & TES & FS --> CHAR_PROG
    FLOOR_VS & FLOOR_FS --> FLOOR_PROG
```

| 文件 | 符号 | 阶段 | 作用 |
|------|------|------|------|
| `shaders.h` | 声明 | — | 6 个 `extern const char*` 全局字符串声明 |
| `shaders.cpp` | `VS_SOURCE` | Vertex | 骨骼关节旋转 + 3 层动画混合 + antiClip 防穿插 |
| | `TCS_SOURCE` | Tess Control | PN 三角形 10 个贝塞尔控制点计算 + 细分级别 |
| | `TES_SOURCE` | Tess Eval | 贝塞尔三角曲面重心坐标求值 + 法线插值 |
| | `FS_SOURCE` | Fragment | Gooch 暖冷色 + Blinn-Phong 高光 + Fresnel + Rim + 骨骼颜色 + Gamma |
| | `FLOOR_VS` | Vertex | 地面简单顶点变换 |
| | `FLOOR_FS` | Fragment | 地面棋盘格颜色 + 深度雾 |

#### 顶点着色器（VS）内部骨骼分组逻辑

```mermaid
graph TD
    VS_ENTRY["VS main()"] --> BONE_SWITCH{inBoneId}

    BONE_SWITCH -->|"0 (TORSO)"| TORSO_PROC["躯干处理<br/>弹跳 + 左右摆动 + 跳跃高度"]

    BONE_SWITCH -->|"1 (HEAD)"| HEAD_PROC["头部处理<br/>弹跳 + 摆动 + 点头旋转"]

    BONE_SWITCH -->|"2-5 (ARMS)"| ARM_PROC["手臂处理<br/>行走摆臂 + 跳跃收臂 + 挥手覆盖"]

    BONE_SWITCH -->|"6-9 (LEGS)"| LEG_PROC["腿部处理<br/>行走摆腿 + 跳跃收腿 + mix 混合"]

    ARM_PROC --> ARM_BLEND{"挥手?"}
    ARM_BLEND -->|"是(右臂)"| WAVE_FULL["挥手完全覆盖<br/>大臂前举1.2rad<br/>小臂正交轴摆动"]
    ARM_BLEND -->|"否"| ARM_ADD["行走+跳跃角度累加<br/>smoothstep 上下臂混合"]

    subgraph COMMON["公共后处理"]
        JUMP_ROT["跳跃旋转 bodyTwist"]
        ANTI_CLIP["antiClip() 防穿插"]
        OUTPUT["输出 pos + norm + tex + bone"]
    end

    TORSO_PROC & HEAD_PROC & ARM_PROC & LEG_PROC --> JUMP_ROT --> ANTI_CLIP --> OUTPUT
```

#### 片段着色器（FS）光照计算流程

```mermaid
graph TD
    FS_ENTRY["FS main()"] --> NORMALIZE["归一化 N, L, V, H"]

    NORMALIZE --> ATTEN["点光源衰减<br/>atten = 1/(1+dist²/r²)"]

    NORMALIZE --> GOOCH["Gooch 暖冷色<br/>t = N·L×0.5+0.5<br/>c = mix(cool, warm, t)"]

    NORMALIZE --> SPEC["高光<br/>pow(N·H, 64) × 0.8"]

    NORMALIZE --> FRESNEL["Fresnel<br/>pow(1-N·V, 3)"]

    NORMALIZE --> RIM["Rim 光<br/>pow(1-N·V, 2)×0.3×atten"]

    GOOCH --> BONECOL["混合骨骼肤色×20%"]

    ATTEN -->|乘算| BONECOL & SPEC & RIM

    BONECOL & SPEC & RIM & FRESNEL --> COMBINE["合成<br/>gooch×(ambient+0.6×atten) + spec + fresnel + rim"]

    COMBINE --> GAMMA["Gamma 校正 ^(1/2.2)"]

    GAMMA --> ALPHA["透明度<br/>0.75+0.25×(1-|N·V|)"]

    ALPHA --> OUTPUT_COLOR["fragColor"]
```

---

### 3.6 渲染层 — `src/render/`

```mermaid
graph TD
    subgraph RENDER_MOD["render 模块 (2 文件)"]
        RENDER_H["render.h<br/>全局状态 + 声明 + 枚举"]
        RENDER_CPP["render.cpp<br/>所有实现"]
    end

    RENDER_H ---|定义| GLOBAL["GlobalState 结构体<br/>GL 资源 + 动画状态 + 渲染参数"]
    RENDER_H ---|定义| ANIM_ENUM["AnimLayer 枚举<br/>IDLE/WALK/JUMP/WAVE"]
    RENDER_H ---|定义| BONE_GROUP["BoneGroup 枚举<br/>6 个骨骼组"]
    RENDER_H ---|定义| FUNC_DECL["函数声明<br/>init/update/render"]

    RENDER_CPP ---|实现| GL_INIT["initGL()<br/>GL 状态初始化"]
    RENDER_CPP ---|实现| CHAR_INIT["initCharacter()<br/>人物 VAO/VBO/EBO"]
    RENDER_CPP ---|实现| FLOOR_INIT["initFloor()<br/>地面 VAO/VBO/EBO"]
    RENDER_CPP ---|实现| LIGHT_INIT["initPointLightMarker()<br/>光源标记"]
    RENDER_CPP ---|实现| SHADER_INIT["initShaders()<br/>编译+链接+uniform"]
    RENDER_CPP ---|实现| UPDATE["updateAnimation()<br/>输入+状态+位移"]
    RENDER_CPP ---|实现| RENDER_SCENE["renderScene()<br/>绘制人物+地面+光源"]
    RENDER_CPP ---|实现| GET_LIGHT["getLightPos()<br/>计算光源轨道"]
```

| 文件 | 函数 / 类型 | 作用 |
|------|-------------|------|
| `render.h` | `GlobalState` | **全局单例** — 管理所有 GL 资源句柄、uniform 位置、动画状态、摄像机参数、人物位置 |
| | `enum AnimLayer` | 动画层优先级：IDLE(0) < WALK(1) < JUMP(2) < WAVE(3) |
| | `enum BoneGroup` | 骨骼分组：ROOT/HEAD/LEFT_ARM/RIGHT_ARM/LEFT_LEG/RIGHT_LEG |
| `render.cpp` | `GlobalState g;` | 全局状态实例 |
| | `compileShader()` | 编译单个 GLSL 着色器，输出错误日志 |
| | `linkProgram()` | 链接多个着色器为程序 |
| | `initGL()` | 设置深度测试 `LEQUAL`、面剔除 `CCW`、混合 `SRC_ALPHA`、清屏色 |
| | `initCharacter()` | 调用 `buildCharacter()` → 创建 VAO/VBO/EBO → 配置 4 个顶点属性 |
| | `initFloor()` | 调用 `buildFloor()` → VAO/VBO/EBO → 仅位置+UV |
| | `initPointLightMarker()` | 创建光源标记的 VAO/VBO（动态更新位置） |
| | `initShaders()` | 编译 4 阶段着色器 → 链接人物程序 → 编译 2 阶段地面程序 → 获取所有 uniform 位置 |
| | `updateAnimation(dt)` | **核心每帧更新** — 时间推进、跳跃/挥手状态机、WASD 输入处理、人物位移 |
| | `getLightPos()` | 计算随时间运动的点光源轨道位置 |
| | `renderScene()` | **核心每帧绘制** — 构建 MVP 矩阵、上传 uniform、绘制人物(细分)/地面/光源 |

#### GlobalState 全局状态结构

```mermaid
graph TB
    subgraph GSTATE["GlobalState 核心字段"]
        RES["GL 资源<br/>charVAO/VBO/EBO<br/>floorVAO/VBO/EBO<br/>pointVAO/VBO<br/>charProgram/floorProgram"]

        UNI["Uniform 位置<br/>uTime_loc<br/>uJumpPhase_loc<br/>uMoveDir_loc<br/>...共 18 个"]

        ANIM["动画状态<br/>time / jumpPhase<br/>moveSpeed / moveDir<br/>jumping / waving / wavePhase<br/>keys[256]"]

        RENDER_PARAM["渲染参数<br/>tessLevel [1,12]<br/>modelRotY<br/>charPosXZ"]

        CAM["摄像机<br/>cameraAngle<br/>cameraDist [2,15]<br/>cameraHeight [1,5]<br/>mouseDragging"]
    end

    RES -->|"每帧 glUseProgram"| UNI
    ANIM -->|"每帧 glUniform1f"| UNI
    CAM -->|"每帧构建 view"| UNI
    RENDER_PARAM -->|"每帧构建 model"| UNI
```

---

## 四、分层调用关系总图

```mermaid
flowchart TB
    subgraph L0["L0 应用层"]
        MAIN["main.cpp"]
    end

    subgraph L1["L1 渲染控制层"]
        RENDER["render.cpp / render.h"]
    end

    subgraph L2["L2 几何生成层"]
        GEOM["geometry.cpp / geometry.h"]
    end

    subgraph L2B["L2 着色器源码层"]
        SHAD["shaders.cpp / shaders.h"]
    end

    subgraph L3["L3 数学基础层"]
        MATH["math/math.h"]
    end

    subgraph L4["L4 系统/GPU"]
        GL["OpenGL 4.1 + GLEW + FreeGLUT"]
        GPU["GPU 管线<br/>VS → TCS → TES → FS"]
    end

    MAIN -->|"回调驱动"| RENDER
    RENDER -->|"buildCharacter/Floor"| GEOM
    RENDER -->|"initShaders"| SHAD
    RENDER -->|"glDrawElements"| GPU
    RENDER -->|"矩阵运算"| MATH
    GEOM -->|"位置/法线"| MATH
    RENDER -->|"初始化"| GL
    GL --> GPU
```

### 函数调用关系矩阵

| 调用者 | 被调用者 | 时机 | 作用 |
|--------|---------|------|------|
| `main()` | `initGL()` | 启动 | 设置 OpenGL 全局状态 |
| `main()` | `initCharacter()` | 启动 | 生成人物网格并上传 GPU |
| `main()` | `initFloor()` | 启动 | 生成地面网格并上传 GPU |
| `main()` | `initPointLightMarker()` | 启动 | 创建光源标记 VAO |
| `main()` | `initShaders()` | 启动 | 编译链接着色器程序 |
| `initCharacter()` | `buildCharacter()` | 启动 | 组装人物各部位网格 |
| `initFloor()` | `buildFloor()` | 启动 | 生成方格地面 |
| `initShaders()` | `compileShader()` ×4 | 启动 | 编译 VS/TCS/TES/FS |
| `initShaders()` | `linkProgram()` | 启动 | 链接成 charProgram |
| `idleCallback()` | `updateAnimation(dt)` | 每帧 | 更新动画状态 + 处理输入 + 移动人物 |
| `displayCallback()` | `renderScene()` | 每帧 | 构建矩阵 + 设置 uniform + 绘制场景 |
| `renderScene()` | `getLightPos()` | 每帧 | 计算光源轨道位置 |
| `keyboardCallback()` | — | 按键时 | 跳跃/挥手/光源/细分/摄像机控制 |
| `mouseCallback()` | — | 点击时 | 左键拖拽 / 滚轮缩放 |
| `mouseMotionCallback()` | — | 拖拽时 | 旋转 / 升降摄像机 |

---

## 五、文件与模块对应汇总

| 文件路径 | 模块 | 角色 | 核心数据/函数 |
|---------|------|------|-------------|
| `src/main.cpp` | 应用入口 | 调度器 | `main()`, 8 个 FreeGLUT 回调 |
| `src/common.h` | 跨平台配置 | 配置 | 平台宏、GLUT/GLEW 头文件顺序 |
| `src/math/math.h` | 数学工具 | 基础库 | `Vec3`, `Mat4` (perspective/lookAt/rotateY/translate/operator*) |
| `src/geometry/geometry.h` | 几何类型 | 接口 | `Vertex`, `BoneId`, `CharacterMesh`, `JointConnectorDef` |
| `src/geometry/geometry.cpp` | 几何生成 | 实现 | 5 个生成器 + `buildCharacter` + `buildFloor` |
| `src/shaders/shaders.h` | 着色器声明 | 接口 | 6 个 `extern const char*` |
| `src/shaders/shaders.cpp` | 着色器源码 | 资源 | 6 段 GLSL (VS/TCS/TES/FS/FLOOR_VS/FLOOR_FS) |
| `src/render/render.h` | 渲染声明 | 接口 | `GlobalState`, `AnimLayer`, `BoneGroup`, 函数声明 |
| `src/render/render.cpp` | 渲染实现 | 核心 | `compileShader`/`linkProgram`, 4 个 init, `updateAnimation`, `renderScene` |

---

## 六、数据流走向

```mermaid
flowchart LR
    subgraph CPU["CPU 端处理"]
        INPUT_P["键盘/鼠标输入"]
        ANIM["updateAnimation()"]
        MATRIX["矩阵计算<br/>proj × view × model"]
        UPLOAD["glUniform 上传"]
    end

    subgraph GPU["GPU 端处理"]
        VS_STAGE["Vertex Shader<br/>骨骼变换 + 层混合"]
        TCS_STAGE["Tess Control<br/>PN 控制点"]
        TES_STAGE["Tess Eval<br/>贝塞尔求值"]
        FS_STAGE["Fragment Shader<br/>Gooch 光照"]
        FB["帧缓冲 → 屏幕"]
    end

    INPUT_P -->|"WASD/鼠标"| ANIM
    ANIM -->|"time/pos/dir"| MATRIX
    ANIM -->|"状态参数"| UPLOAD
    MATRIX -->|"mvp/model"| UPLOAD
    UPLOAD --> VS_STAGE

    VBO["VBO (GPU 显存)<br/>顶点数据"] --> VS_STAGE
    VS_STAGE --> TCS_STAGE --> TES_STAGE --> FS_STAGE --> FB
```

---

## 七、关键流程时序

```mermaid
sequenceDiagram
    participant User as 用户输入
    participant Main as main.cpp
    participant Render as render.cpp
    participant Geom as geometry.cpp
    participant GPU as GPU 管线

    Note over Main,GPU: 初始化阶段
    Main->>Render: initGL()
    Main->>Render: initCharacter()
    Render->>Geom: buildCharacter()
    Geom-->>Render: CharacterMesh
    Render->>GPU: VAO/VBO/EBO 上传
    Main->>Render: initShaders()
    Render->>GPU: 编译 VS+TCS+TES+FS

    Note over User,GPU: 运行阶段 (每帧)
    User->>Main: 按键/鼠标事件
    Main->>Render: updateAnimation(dt)
    Render->>Render: 状态机 + WASD 输入
    Main->>Render: renderScene()
    Render->>Render: 构建 MVP 矩阵
    Render->>GPU: glUniform (时间/方向/光源...)
    Render->>GPU: glDrawElements(GL_PATCHES)
    GPU->>GPU: VS: 骨骼动画 + antiClip
    GPU->>GPU: TCS: PN 控制点
    GPU->>GPU: TES: 贝塞尔曲面
    GPU->>GPU: FS: Gooch 着色
    GPU-->>Render: 帧缓冲完成
    Render->>Render: glutSwapBuffers()
```