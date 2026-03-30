#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

// 全局效果
bool doubleScoreActive = false;
float doubleScoreTimer = 0.0f;

int main() {

    bool paused = false;
    bool ballLaunched = false;

    int score = 0;
    int lives = 3;
    bool gameOver = false;
    bool win = false;

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Breakout");

    Paddle paddle(350.0f, 550.0f, 100.0f, 20.0f);

    std::vector<Ball> balls;
    std::vector<Brick> bricks;

    float brickWidth = 80.0f;
    float brickHeight = 25.0f;

    int rows = 5;
    int cols = 8;

    // 初始化砖块
    auto ResetBricks = [&]() {
        bricks.clear();
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {

                BrickType type = NORMAL;

                if ((r + c) % 7 == 0)
                    type = SPLIT;
                else if ((r + c) % 5 == 0)
                    type = DOUBLE_SCORE;

                bricks.emplace_back(
                    40.0f + c * 90.0f,
                    80.0f + r * 40.0f,
                    brickWidth,
                    brickHeight,
                    type
                );
            }
        }
    };

    // 初始化球
    auto ResetBall = [&]() {
        balls.clear();
        balls.push_back(Ball(
            {paddle.GetRect().x + paddle.GetRect().width / 2.0f,
             paddle.GetRect().y - 10},
            4.0f,
            90.0f,
            10.0f
        ));
        ballLaunched = false;
    };

    ResetBricks();
    ResetBall();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        // 输入
        if (IsKeyPressed(KEY_P)) paused = !paused;

        if (IsKeyPressed(KEY_R)) {
            score = 0;
            lives = 3;
            gameOver = false;
            win = false;
            paused = false;
            doubleScoreActive = false;

            ResetBricks();
            ResetBall();
        }

        if (!ballLaunched && IsKeyPressed(KEY_SPACE)) {
            ballLaunched = true;
        }

        // =================================
        // 游戏逻辑
        // =================================
        if (!gameOver && !win && !paused) {

            // 板子移动
            if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5.0f);
            if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5.0f);

            // 未发射 → 跟随板子
            if (!ballLaunched) {
                for (auto& ball : balls) {
                    ball.SetPosition({
                        paddle.GetRect().x + paddle.GetRect().width / 2.0f,
                        paddle.GetRect().y - ball.GetRadius()
                    });
                }
            }

            std::vector<Ball> newBalls;

            for (auto& ball : balls) {

                if (ballLaunched)
                    ball.Move();

                ball.BounceEdge(screenWidth, screenHeight);

                // 板子碰撞（真实反射）
                if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle.GetRect())) {

                    Rectangle rect = paddle.GetRect();

                    ball.SetPosition({ ball.GetPosition().x, rect.y - ball.GetRadius() });

                    float paddleCenter = rect.x + rect.width / 2.0f;

                    float offset = (ball.GetPosition().x - paddleCenter) / (rect.width / 2.0f);

                    // 真实反射
                    ball.Reflect({0, -1});

                    // 微调方向（防纯竖）
                    Vector2 v = ball.GetVelocity();
                    v.x += offset * 2.0f;
                    ball.SetVelocity(v);
                }

                // 砖块碰撞
                for (auto& brick : bricks) {

                    if (!brick.IsDestroyed() &&
                        CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), brick.GetRect())) {

                        int addScore = (brick.GetType() == NORMAL) ? 10 : 20;
                        if (doubleScoreActive) addScore *= 2;

                        score += addScore;

                        if (brick.GetType() == SPLIT) {
                            newBalls.push_back(Ball(ball.GetPosition(), 4.0f, 30.0f, 10.0f));
                            newBalls.push_back(Ball(ball.GetPosition(), 4.0f, -30.0f, 10.0f));
                        }

                        if (brick.GetType() == DOUBLE_SCORE) {
                            doubleScoreActive = true;
                            doubleScoreTimer = 10.0f;
                        }

                        brick.Destroy();

                        ball.Reflect({0, 1}); // 向上反弹
                        break;
                    }
                }
            }

            balls.insert(balls.end(), newBalls.begin(), newBalls.end());

            // 删除掉落球
            for (auto it = balls.begin(); it != balls.end();) {
                if (it->GetPosition().y > screenHeight)
                    it = balls.erase(it);
                else
                    ++it;
            }

            if (balls.empty()) {
                lives--;

                if (lives <= 0) {
                    gameOver = true;
                } else {
                    ResetBall();
                }
            }

            // 胜利检测
            bool allDestroyed = true;
            for (auto& brick : bricks) {
                if (!brick.IsDestroyed()) {
                    allDestroyed = false;
                    break;
                }
            }
            if (allDestroyed) win = true;

            // 双倍分数计时
            float dt = GetFrameTime();
            if (doubleScoreActive) {
                doubleScoreTimer -= dt;
                if (doubleScoreTimer <= 0)
                    doubleScoreActive = false;
            }
        }

        // =================================
        // 渲染
        // =================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        paddle.Draw();

        for (auto& ball : balls)
            ball.Draw();

        for (auto& brick : bricks)
            brick.Draw();

        DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
        DrawText(TextFormat("Lives: %d", lives), 650, 10, 20, BLACK);

        if (!ballLaunched)
            DrawText("Press SPACE to launch", 260, 300, 20, GRAY);

        if (paused)
            DrawText("PAUSED", 340, 250, 40, DARKGRAY);

        if (doubleScoreActive)
            DrawText("DOUBLE SCORE!", 300, 50, 20, PURPLE);

        if (gameOver)
            DrawText("GAME OVER", 280, 250, 40, RED);

        if (win)
            DrawText("YOU WIN!", 300, 250, 40, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
