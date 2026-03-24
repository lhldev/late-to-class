#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "raylib.h"

struct Obstacle {
    Rectangle rect;
    int lane;
    Color color;
};

struct PowerUp {
    Rectangle rect;
    int lane;
};

static float LaneToY(int lane, float roadTop, float laneHeight, float objectHeight) {
    const int laneIndex = lane + 1;
    return roadTop + laneHeight * laneIndex + (laneHeight - objectHeight) * 0.5f;
}

int main(void) {
    const int baseScreenWidth = 1120;
    const int baseScreenHeight = 680;
    const float playerSize = 56.0f;

    const float baseAcceleration = 0.001f;
    const float baseSpawnTimeObstacle = 1.0f;
    const float baseSpawnTimePowerup = 3.4f;
    const float playerInvulnerableDuration = 2.3f;

    bool died = false;
    float score = 0.0f;
    float velocity = 1.0f;
    int player_row = 0;

    std::vector<Obstacle> obstacles;
    std::vector<PowerUp> powerups;

    float obstacleSpawnTimer = 0.0f;
    float powerupSpawnTimer = 0.0f;
    float invulnerableTimer = 0.0f;
    float elapsedTime = 0.0f;
    float laneDashOffset = 0.0f;

    InitWindow(baseScreenWidth, baseScreenHeight, "Late To Class!!");
    SetTargetFPS(60);

    const std::array<Color, 8> carPalette = {
        Color{255, 45, 45, 255}, Color{17, 102, 255, 255}, Color{255, 145, 30, 255}, Color{32, 196, 84, 255},
        Color{170, 67, 255, 255}, Color{255, 211, 61, 255}, Color{242, 82, 121, 255}, Color{31, 187, 185, 255}};

    while (!WindowShouldClose()) {
        const float delta_time = GetFrameTime();
        const int screenWidth = baseScreenWidth;
        const int screenHeight = baseScreenHeight;

        const float grassBandHeight = static_cast<float>(screenHeight) * 0.15f;
        const float roadTop = grassBandHeight;
        const float roadHeight = static_cast<float>(screenHeight) - (grassBandHeight * 2.0f);
        const float laneHeight = roadHeight / 3.0f;
        const float playerX = static_cast<float>(screenWidth) * 0.14f;

        if (!died) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                if (player_row < 1) player_row++;
            } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                if (player_row > -1) player_row--;
            }

            elapsedTime += delta_time;
            // Nonlinear acceleration curve to increase difficulty over time.
            const float nonlinearAcceleration = baseAcceleration + 0.00070f * powf(elapsedTime, 1.25f);
            velocity += nonlinearAcceleration * delta_time;
            score += velocity * delta_time * 100.0f;

            obstacleSpawnTimer += delta_time;
            powerupSpawnTimer += delta_time;

            const float laneSpeed = 260.0f + velocity * 110.0f;
            const float obstacleSpawnInterval = baseSpawnTimeObstacle / (1.0f + 0.22f * velocity + 0.06f * sqrtf(velocity));
            const float powerupSpawnInterval = baseSpawnTimePowerup / (1.0f + velocity * 0.08f);
            laneDashOffset += laneSpeed * delta_time;
            if (laneDashOffset > 120.0f) laneDashOffset = fmodf(laneDashOffset, 120.0f);

            if (obstacleSpawnTimer >= obstacleSpawnInterval) {
                obstacleSpawnTimer = 0.0f;
                const int lane = GetRandomValue(-1, 1);
                const float carHeight = laneHeight * 0.56f;
                const float carWidth = carHeight * 1.34f;
                obstacles.push_back({Rectangle{static_cast<float>(screenWidth) + 40.0f, 0.0f, carWidth, carHeight},
                                     lane,
                                     carPalette[GetRandomValue(0, static_cast<int>(carPalette.size()) - 1)]});
            }

            if (powerupSpawnTimer >= powerupSpawnInterval) {
                powerupSpawnTimer = 0.0f;
                if (GetRandomValue(0, 99) < 55) {
                    const int lane = GetRandomValue(-1, 1);
                    const float orbSize = laneHeight * 0.31f;
                    powerups.push_back({Rectangle{static_cast<float>(screenWidth) + 40.0f, 0.0f, orbSize, orbSize}, lane});
                }
            }

            for (Obstacle &o : obstacles) o.rect.x -= laneSpeed * delta_time;
            for (PowerUp &p : powerups) p.rect.x -= laneSpeed * delta_time * 0.9f;

            obstacles.erase(
                std::remove_if(obstacles.begin(), obstacles.end(),
                               [](const Obstacle &o) { return o.rect.x + o.rect.width < 0.0f; }),
                obstacles.end());
            powerups.erase(
                std::remove_if(powerups.begin(), powerups.end(),
                               [](const PowerUp &p) { return p.rect.x + p.rect.width < 0.0f; }),
                powerups.end());

            Rectangle playerRect{
                playerX,
                LaneToY(player_row, roadTop, laneHeight, playerSize),
                playerSize,
                playerSize};

            if (invulnerableTimer > 0.0f) invulnerableTimer -= delta_time;

            for (const Obstacle &o : obstacles) {
                Rectangle collider = o.rect;
                collider.y = LaneToY(o.lane, roadTop, laneHeight, o.rect.height);
                if (CheckCollisionRecs(playerRect, collider) && invulnerableTimer <= 0.0f) {
                    died = true;
                    break;
                }
            }

            for (size_t i = 0; i < powerups.size();) {
                Rectangle orbCollider = powerups[i].rect;
                orbCollider.y = LaneToY(powerups[i].lane, roadTop, laneHeight, powerups[i].rect.height);
                if (CheckCollisionRecs(playerRect, orbCollider)) {
                    // Power-up grants temporary invulnerability and score bonus.
                    invulnerableTimer = playerInvulnerableDuration;
                    score += 180.0f;
                    powerups.erase(powerups.begin() + static_cast<int>(i));
                } else {
                    i++;
                }
            }
        } else {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                died = false;
                score = 0.0f;
                velocity = 1.0f;
                player_row = 0;
                obstacleSpawnTimer = 0.0f;
                powerupSpawnTimer = 0.0f;
                invulnerableTimer = 0.0f;
                elapsedTime = 0.0f;
                laneDashOffset = 0.0f;
                obstacles.clear();
                powerups.clear();
            }
        }

        BeginDrawing();
        ClearBackground(Color{16, 190, 105, 255});

        DrawRectangle(0, static_cast<int>(roadTop), screenWidth, static_cast<int>(roadHeight), Color{124, 124, 124, 255});
        DrawRectangle(0, 0, screenWidth, static_cast<int>(grassBandHeight), Color{9, 191, 103, 255});
        DrawRectangle(0, static_cast<int>(screenHeight - grassBandHeight), screenWidth, static_cast<int>(grassBandHeight),
                      Color{9, 191, 103, 255});

        for (int i = -1; i <= 7; i++) {
            const float x = static_cast<float>(i) * 190.0f;
            DrawCircle(static_cast<int>(x + 80.0f), static_cast<int>(grassBandHeight * 0.5f), 34.0f, Color{63, 128, 49, 255});
            DrawCircle(static_cast<int>(x + 80.0f), static_cast<int>(screenHeight - grassBandHeight * 0.5f), 34.0f,
                       Color{63, 128, 49, 255});
        }

        for (int split = 1; split <= 2; split++) {
            const int y = static_cast<int>(roadTop + laneHeight * split);
            for (float x = -laneDashOffset; x < static_cast<float>(screenWidth) + 120.0f; x += 120.0f) {
                DrawRectangle(static_cast<int>(x), y - 6, 78, 12, RAYWHITE);
            }
        }

        const Rectangle playerRect{
            playerX,
            LaneToY(player_row, roadTop, laneHeight, playerSize),
            playerSize,
            playerSize};
        const Color ringColor = (invulnerableTimer > 0.0f) ? Color{80, 235, 255, 255} : Color{133, 77, 255, 255};
        DrawCircleLines(static_cast<int>(playerRect.x + playerRect.width * 0.5f), static_cast<int>(playerRect.y + playerRect.height * 0.5f),
                        playerRect.width * 0.45f, ringColor);
        DrawCircle(static_cast<int>(playerRect.x + playerRect.width * 0.5f), static_cast<int>(playerRect.y + playerRect.height * 0.5f),
                   playerRect.width * 0.24f, Color{86, 86, 86, 255});

        for (const Obstacle &o : obstacles) {
            Rectangle carRect = o.rect;
            carRect.y = LaneToY(o.lane, roadTop, laneHeight, o.rect.height);
            const float wheelR = o.rect.height * 0.18f;
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.20f), static_cast<int>(carRect.y - wheelR * 0.2f), wheelR, BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.80f), static_cast<int>(carRect.y - wheelR * 0.2f), wheelR, BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.20f), static_cast<int>(carRect.y + carRect.height + wheelR * 0.2f), wheelR,
                       BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.80f), static_cast<int>(carRect.y + carRect.height + wheelR * 0.2f), wheelR,
                       BLACK);
            DrawRectangleRounded(carRect, 0.10f, 4, o.color);
        }

        for (const PowerUp &p : powerups) {
            const float y = LaneToY(p.lane, roadTop, laneHeight, p.rect.height);
            DrawCircle(static_cast<int>(p.rect.x + p.rect.width / 2.0f), static_cast<int>(y + p.rect.height / 2.0f), p.rect.width * 0.5f,
                       Color{86, 207, 94, 255});
            DrawText("+", static_cast<int>(p.rect.x + p.rect.width * 0.28f), static_cast<int>(y - p.rect.height * 0.08f),
                     static_cast<int>(p.rect.height * 0.92f), DARKGREEN);
        }

        DrawText("LATE TO CLASS!!", 24, 18, 32, RAYWHITE);
        DrawText(TextFormat("Score: %i", static_cast<int>(score)), 24, 58, 26, Color{255, 241, 170, 255});
        DrawText(TextFormat("Speed: %.2fx", velocity), 24, 88, 22, RAYWHITE);
        DrawText("Move: UP/DOWN or W/S", 24, 114, 20, RAYWHITE);
        DrawText(TextFormat("Window: %ix%i", screenWidth, screenHeight), 24, 138, 20, RAYWHITE);

        if (invulnerableTimer > 0.0f) {
            DrawText(TextFormat("Shield: %.1fs", invulnerableTimer), 24, 164, 22, Color{104, 237, 255, 255});
        }

        if (died) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 140});
            DrawText("YOU DIED", screenWidth / 2 - 116, screenHeight / 2 - 74, 50, RED);
            DrawText(TextFormat("Final Score: %i", static_cast<int>(score)), screenWidth / 2 - 114, screenHeight / 2 - 16, 30,
                     RAYWHITE);
            DrawText("Press R / ENTER / Left Click to Restart", screenWidth / 2 - 206, screenHeight / 2 + 36, 24, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
