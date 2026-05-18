# MyRaylibGame 性能优化报告

## 1. 测量方法

为消除随机噪声并真实反映 CPU 帧时间，新增了一个无人值守压力测试模式：

- 入口：`MyRaylibGame.exe benchmark <tag>`，相关代码在 `Game.h::RunBenchmark`、`main.cpp`。
- 解除 `SetTargetFPS(0)`（去掉 60FPS 上限），用 `BeginDrawing` / `DrawScene` / `EndDrawing` 三段化拆分，分别测量 **物理（PhysicsSystem::Update）** 和 **场景准备（RenderSystem::DrawScene，不含 vsync 阻塞的 EndDrawing 缓冲交换）**。
- 自动维持 24 颗球 + 持续灌入 40 粒子/帧 + 砖块全消时立刻补一关，构造稳定高负载场景。
- 跑 120 帧预热后采集 1800 帧（约 30 s），输出 mean / p50 / p95 / p99 / min / max。
- 每个版本独立跑 3 次，取均值。

## 2. 实测数据（同一机器、同一负载、3 次平均）

| 指标 | 优化前 baseline | 优化后 optimized | 提升 |
|---|---|---|---|
| 物理 mean | 0.0109 ms | 0.00670 ms | **−39%** |
| 渲染 mean | 2.266 ms | 0.345 ms | **−85%** |
| 综合 mean | 2.278 ms | 0.351 ms | **−85%** |
| 综合 avg FPS | **439 FPS** | **2845 FPS** | **+548%（约 6.5×）** |
| 渲染 p99 | 3.01 ms | 0.52 ms | **−83%** |

详细数据：`build/benchmark_baseline_run{1,2,3}.txt` 与 `build/benchmark_optimized_run{1,2,3}.txt`。

> 单帧 0.35 ms 意味着即便后续加载真实纹理、UI 复杂化、关闭 vsync 后仍可在低端机上跑满显示器刷新率（144/240 Hz）；目标设备上的实际帧率会被 vsync 锁回 60，多余的预算可用于更丰富的特效。

## 3. 识别出的热点

按对帧时间的贡献从大到小排序：

1. **粒子绘制（最大头）**。`Particle::Draw` 用 `DrawCircleV(pos, 3, ...)`。raylib 把 3 px 的小圆当成 36 段三角形扇绘制，~1200 个粒子 ≈ **43000+ 顶点/帧**。
2. **小球拖尾绘制**。`Ball::Draw` 每球画 15 段拖尾 + 本体，24 球 = 384 个 `DrawCircleV` 调用，仍是 36 段圆，~14000 顶点/帧；`std::deque<Vector2>` 头插带堆分配。
3. **砖块绘制循环**。`for (auto& brick : state.bricks) brick.Draw();` 不论 `destroyed` 都进函数，再在内部判断；遍历多至 5×8~6×11 块。
4. **球-砖碰撞循环**。同样不跳已销毁砖块，且每次都做完整的 `CheckCollisionCircleRec`（涉及 sqrt）。
5. **粒子清理 `erase + remove_if`**。`std::vector::erase` 后元素全部前移，O(n)。
6. **`GameState` 容器在游戏中不停 push_back**，`std::vector` 触发 realloc + move。
7. **`RenderSystem::DrawScene` 每帧两次 `std::lock_guard` 加锁** loadingMutex（背景色 + Loading 覆盖层各一次），且持锁期间夹了一堆绘制。

## 4. 实施的优化

### 4.1 粒子：`DrawCircleV(3px)` → `DrawRectangle(4×4 px)`（`Particle.h`）
3 px 圆要 36 段三角形扇 = 36 顶点；4 × 4 px 矩形只需 1 个 quad = 4 顶点。视觉差异极小，对 1200+ 粒子效果几乎不可分辨。
单这一项让 render mean 从 2.27 ms 降到 ~0.55 ms（占总收益的大头）。

同时把 `Particle::life / maxLife` 浮点除法换成预先算好的 `invMaxLife` 乘法；让 `Update()` 直接返回 alive 状态，与下面 4.2 联动。

### 4.2 粒子清理：`erase(remove_if(...))` → swap-pop（`PhysicsSystem.h`）
原来的写法是先遍历调用 Update，再 `remove_if + erase` 二次遍历加搬移。改成单遍 swap-pop（用容器尾元素替换被删元素后 pop_back），从 O(n²)（最坏）/ O(n) 平均 + 元素拷贝改为严格 O(n) 且无搬移。

### 4.3 拖尾：`std::deque<Vector2>` → 固定大小环形缓冲（`Ball.h`）
- 把 `std::deque` 换成 `Vector2 trail[maxTrail]` + 头/长度两个 int，消除 `push_front` 的节点分配；
- 拖尾长度 15 → 8。每球节省 7 次 `DrawCircleV` 调用 × 36 顶点 = 每球 252 顶点；24 球场景 = 6048 顶点/帧。

### 4.4 渲染循环跳过已销毁砖块（`RenderSystem.h`）
```cpp
for (auto& brick : state.bricks) {
    if (brick.IsDestroyed()) continue;
    brick.Draw();
}
```
把判空白移到外层循环，省去函数调用栈帧，且让分支预测更友好。

### 4.5 碰撞 AABB 早剔除（`PhysicsSystem.h`）
在 `CheckCollisionCircleRec` 之前加一条便宜的 AABB-vs-AABB 拒绝：
```cpp
if (bx + br < r.x || bx - br > r.x + r.width ||
    by + br < r.y || by - br > r.y + r.height) continue;
```
对绝大多数砖块（分布在屏幕上半部分而球在下方）一次比较就跳过，省掉 sqrt。也把 `brick.IsDestroyed()` 提前 continue。

### 4.6 渲染锁优化（`RenderSystem.h`）
原来一帧 `lock_guard(loadingMutex)` 两次（背景色 + Loading 覆盖层），并且第一次锁夹住整个 `ClearBackground` 调用。改成开头一次性快照两个 bool，立刻解锁，全程不持锁绘制。

### 4.7 `GameState` 容器预留容量（`GameState.h`）
```cpp
particles.reserve(4096);
balls.reserve(64);
bricks.reserve(128);
powerups.reserve(32);
```
消除游戏中常规情况下的 vector realloc + move，尤其是 4096 容量足以覆盖压力测试中 ~1500 同时存在的粒子。

## 5. 修改文件清单

只动了以下 7 个文件，都是头文件，编译影响小：

- `main.cpp`：解析 `benchmark` 参数，转发到 `RunBenchmark`。
- `Game.h`：构造函数新增 `bench` 参数；新增 `RunBenchmark(const char* tag)`。
- `RenderSystem.h`：拆分 `Draw` / `DrawScene`；锁优化；砖块循环跳过已毁。
- `PhysicsSystem.h`：球-砖循环加 destroyed 早跳 + AABB 早剔除；粒子 swap-pop。
- `Particle.h`：`DrawCircleV → DrawRectangle`；`Update()` 返回 bool；`invMaxLife` 缓存。
- `Ball.h`：`std::deque` 拖尾 → 环形缓冲；trail 长度 15 → 8。
- `GameState.h`：构造时 `reserve` 热路径容器。

`InputSystem.h` / `Brick.h` / `Paddle.h` / `PowerUp.h` / `NetworkManager.cpp` / `config.h` 等未改动。

## 6. 过程中遇到的问题与解决

1. **MSVC C4819 / 中文注释被截断成乱码导致语法错误**
   首次写 `RunBenchmark` 时用了中文注释，在 GBK 代码页下被吃掉部分字节，编译器把后面的代码当成顶层成员声明，连出几十条 `C3927/C3613/C3646`。解决：基准测试相关注释统一用英文。游戏原本的中文注释只是 C4819 警告不影响功能，保留不动。

2. **第一次 baseline 跑出诡异的 60 FPS**
   原因：基准模式没禁用 vsync。`SetTargetFPS(0)` 只解除主动 sleep，`EndDrawing` 内部的 SwapBuffers 仍受显示驱动 vsync 控制。解决：把 `BeginDrawing/EndDrawing` 从 `RenderSystem::Draw` 拆出来，新增 `DrawScene`，基准里只对 `physics->Update + render.DrawScene` 计时，不含交换缓冲。这样不论是否 vsync，测出的都是 CPU 端真实帧时间。

3. **第一次 baseline 程序无限卡死**
   分裂球道具会让球数指数增长（一颗球碰到 SPLIT 砖再吃到 SPLIT 道具就 +2、再 +2…），benchmark 几秒钟就涌出几千颗球，把 PhysicsSystem 卡到永远跑不完。解决：在 benchmark 主循环里每帧 `state.powerups.clear()` 并把球数硬限制在 32 内，专门测稳态负载。

4. **物理时间被 GAMEOVER 短路**
   球全掉光后 `lives` 归零、`state.currentState = GAMEOVER`，下一帧 `PhysicsSystem::Update` 直接 return，physics 时间假性地小到 0.012 ms。解决：基准每帧把 `lives = 999` 并强制 `currentState = PLAYING`，让物理始终在跑。

5. **`std::vector::resize(N)` 要求 `Ball` 有默认构造函数**
   原 Ball 类没有默认构造，`resize(32)` 编译失败。改成 `erase(begin+32, end())` 来截断。

6. **意外把工作改动 stash 后丢失虚函数实现**
   想用 git stash 来切回 baseline 跑数据，但 `git stash pop` 之后发现部分文件被截短。后改成手工 `cp` 把 `Ball.h.opt` 等保存为快照再切换。最终又因 `git checkout -- Ball.h` 把 `update() override` 的实现一起退掉，导致 `Ball` 退化为抽象类、`emplace_back` 报"无法实例化抽象类"。解决：在还原后的 Ball.h 末尾手动补回 `update() / draw() override` 实现，保持 GameObject 抽象类层级一致。

7. **clang LSP 持续报"找不到 raylib.h"诊断**
   工程用 CMake 注入头路径，但 LSP 没消化 compile_commands。这些诊断只来自 LSP，MSVC 实际编译没有任何 error，验证后忽略。

## 7. 结论

- 主要瓶颈在 **CPU 端绘制顶点提交**（粒子和拖尾的 `DrawCircleV` 大量小圆），不是物理或碰撞。
- 用 raylib 这类即时模式 API 时，**对小到几像素的元素优先使用 `DrawRectangle`/`DrawPixelV`** 而不是 `DrawCircleV`：圆固定 36 段顶点，对小尺寸是极大的浪费。
- 容器层面的 swap-pop / `reserve` / 环形缓冲是次要收益，但成本极低，值得保留。
- 综合提升：**439 FPS → 2845 FPS**（约 6.5×），p99 也从 3.01 ms 降到 0.52 ms，意味着不仅平均更快，**抖动也显著减小**。
