#include <cassert>
#include "Ball.h"
#include "Brick.h"
#include "GameState.h"
#include "PhysicsSystem.h"

// ==========================
// 碰撞测试
// ==========================
void TestCollision() {

    Ball ball({100, 100}, 4.0f, 90.0f, 10.0f);
    Brick brick(90, 80, 40, 20, NORMAL);

    bool collision = CheckCollisionCircleRec(
        ball.GetPosition(),
        ball.GetRadius(),
        brick.GetRect()
    );

    assert(collision == true);
}

// ==========================
// 计分测试
// ==========================
void TestScore() {

    int score = 0;

    Brick normal(0,0,10,10,NORMAL);
    Brick special(0,0,10,10,SPLIT);

    score += (normal.GetType() == NORMAL) ? 10 : 20;
    assert(score == 10);

    score += (special.GetType() == NORMAL) ? 10 : 20;
    assert(score == 30);
}

// ==========================
// 关卡砖块生成测试
// ==========================
void TestLevelBricks() {
    GameState state;
    PhysicsSystem physics(800, 600);
    
    // 测试Level 1
    state.currentLevel = 1;
    physics.ResetBricks(state);
    int level1Bricks = state.bricks.size();
    printf("Level 1: %d bricks (%dx%d)\n", level1Bricks, state.rows, state.cols);
    
    // 测试Level 2
    state.currentLevel = 2;
    physics.ResetBricks(state);
    int level2Bricks = state.bricks.size();
    printf("Level 2: %d bricks (%dx%d)\n", level2Bricks, state.rows, state.cols);
    
    // 测试Level 3
    state.currentLevel = 3;
    physics.ResetBricks(state);
    int level3Bricks = state.bricks.size();
    printf("Level 3: %d bricks (%dx%d)\n", level3Bricks, state.rows, state.cols);
    
    // 验证结果
    assert(level1Bricks == 40); // 5x8 = 40
    assert(level2Bricks == 66); // 6x11 = 66
    assert(level3Bricks == 66); // 6x11 = 66
}

// ==========================
// 小球运动测试
// ==========================
void TestBallMovement() {
    GameState state;
    PhysicsSystem physics(800, 600);

    // 初始化小球
    state.balls.emplace_back(Vector2{400, 300}, 4.0f, 90.0f, 10.0f);
    state.ballLaunched = true;
    state.currentState = GameStatus::PLAYING;

    // 记录初始位置
    Vector2 initialPos = state.balls[0].GetPosition();

    // 更新物理
    physics.Update(state);

    // 检查位置是否改变
    Vector2 newPos = state.balls[0].GetPosition();

    // 小球应该向上移动 (角度90度，y减少)
    assert(newPos.y < initialPos.y);
    assert(newPos.x == initialPos.x); // 90度是垂直向上
}

// ==========================
int main() {
    TestCollision();
    TestScore();
    TestLevelBricks();
    TestBallMovement();

    return 0;
}