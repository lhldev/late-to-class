#include <cmath>
#include <vector>

#include "raylib.h"

using namespace std;

/*
 *
 * COMMENTS
 *
 * Project Name: Late To Class!!
 * File Name:    main.cpp
 *
 * Description:
 * A 2D top-down arcade dodging game developed utilizing C++ and the Raylib library.
 * The objective is to simulate a student rushing to class, navigating through
 * three distinct lanes of traffic to avoid oncoming vehicles.
 *
 * Instructions & Controls:
 * - UP ARROW or 'W' Key   : Move the player character one lane up.
 * - DOWN ARROW or 'S' Key : Move the player character one lane down.
 * Gameplay Elements:
 * - Vehicles (Obstacles)  : Colliding with a vehicle ends the game immediately.
 * - Green Orbs (Power-ups): Collecting an orb grants a temporary shield
 * (invulnerability for 2.3 seconds) and awards 3000 points.
 * - Blue Orbs (Slowdown)  : Collecting an orb reduces the current velocity
 * and awards 1500 points.
 *
 * Post-Game:
 * - Restart: Press 'R', 'ENTER', or Left Mouse Click on the "YOU DIED" screen
 * to reset the simulation and try again.
 *
 */

// Lane goes from -1, 0, 1
// -1 represents the top row
float LaneToY(int lane, float roadTop, float laneHeight, float objectHeight) {
    const int laneIndex = lane + 1;
    return roadTop + laneHeight * laneIndex + (laneHeight - objectHeight) * 0.5f;
}

int main(void) {
    const int screenWidth = 1120;
    const int screenHeight = 680;
    const float playerSize = 56.0f;

    const float baseAcceleration = 40.0f;
    const float baseSpawnTimeObstacle = 1.0f;
    const float baseSpawnTimePowerup = 3.4f;
    const float baseSpawnTimeSlowdown = 4.5f;
    const float playerInvulnerableDuration = 2.3f;
    const float slowdownAmount = 80.0f;
    const float minVelocity = 200.0f;

    // SOUND EFFECT AND BACKGROUND MUSIC

    InitAudioDevice();
    Music bgMusic = LoadMusicStream("assets/audio/backg.mp3");
    SetMusicVolume(bgMusic, 0.3f);
    PlayMusicStream(bgMusic);

    Sound deathSound = LoadSound("assets/audio/death.wav");
    Sound shieldSound = LoadSound("assets/audio/shield.wav");
    if (shieldSound.frameCount == 0) {
        TraceLog(LOG_ERROR, "SHIELD SOUND FAILED TO LOAD! Check path: assets/audio/shield.wav");
    }

    bool deathSoundPlayed = false;
    bool died = false;
    float score = 0.0f;
    float velocity = 370.0f;
    int player_row = 0;
    vector<Rectangle> obstacleRects;
    vector<Color> obstacleColors;
    vector<Rectangle> powerups;
    vector<Rectangle> slowdownPowerups;
    float obstacleSpawnTimer = 0.0f;
    float powerupSpawnTimer = 0.0f;
    float slowdownSpawnTimer = 0.0f;
    float invulnerableTimer = 0.0f;
    float laneOffset = 0.0f;

    InitWindow(screenWidth, screenHeight, "Late To Class!!");
    SetTargetFPS(60);

    const int carPaletteSize = 8;
    const Color carPalette[carPaletteSize] = {
        Color{255, 45, 45, 255}, Color{17, 102, 255, 255},
        Color{255, 145, 30, 255}, Color{32, 196, 84, 255},
        Color{170, 67, 255, 255}, Color{255, 211, 61, 255},
        Color{242, 82, 121, 255}, Color{31, 187, 185, 255}};

    const float grassBandHeight = static_cast<float>(screenHeight) * 0.15f;
    const float roadTop = grassBandHeight;
    const float roadHeight = static_cast<float>(screenHeight) - (grassBandHeight * 2.0f);
    const float laneHeight = roadHeight / 3.0f;
    const float playerX = static_cast<float>(screenWidth) * 0.14f;

    float delta_time;
    while (!WindowShouldClose()) {
        delta_time = GetFrameTime();

        const Rectangle playerRect{playerX, LaneToY(player_row, roadTop, laneHeight, playerSize), playerSize, playerSize};

        if (!died) {
            UpdateMusicStream(bgMusic);
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                if (player_row > -1) player_row--;
            } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                if (player_row < 1) player_row++;
            }

            // Nonlinear acceleration curve to increase difficulty over time.
            const float nonlinearAcceleration = baseAcceleration * exp(-velocity * 0.004f);
            velocity += nonlinearAcceleration * delta_time;
            score += velocity * delta_time;

            obstacleSpawnTimer += delta_time;
            powerupSpawnTimer += delta_time;
            slowdownSpawnTimer += delta_time;
            const float obstacleSpawnInterval = baseSpawnTimeObstacle / velocity * 300.0f;
            const float powerupSpawnInterval = baseSpawnTimePowerup / velocity * 800.0f;
            const float slowdownSpawnInterval = baseSpawnTimeSlowdown / velocity * 800.0f;

            laneOffset += velocity * 0.8f * delta_time;

            if (obstacleSpawnTimer >= obstacleSpawnInterval) {
                obstacleSpawnTimer = 0.0f;
                const int lane = GetRandomValue(-1, 1);
                const float carHeight = laneHeight * 0.56f;
                const float carWidth = carHeight * 1.34f;

                obstacleRects.push_back(Rectangle{static_cast<float>(screenWidth) + 40.0f,
                                                  LaneToY(lane, roadTop, laneHeight, carHeight), carWidth, carHeight});
                obstacleColors.push_back(carPalette[GetRandomValue(0, carPaletteSize - 1)]);
            }

            if (powerupSpawnTimer >= powerupSpawnInterval) {
                powerupSpawnTimer = 0.0f;
                if (GetRandomValue(0, 99) < 55) {
                    const int lane = GetRandomValue(-1, 1);
                    const float orbSize = laneHeight * 0.31f;
                    powerups.push_back({Rectangle{static_cast<float>(screenWidth) + 40.0f,
                                                  LaneToY(lane, roadTop, laneHeight, orbSize), orbSize, orbSize}});
                }
            }

            if (slowdownSpawnTimer >= slowdownSpawnInterval) {
                slowdownSpawnTimer = 0.0f;
                if (GetRandomValue(0, 99) < 40) {  // 40% chance to spawn
                    const int lane = GetRandomValue(-1, 1);
                    const float orbSize = laneHeight * 0.31f;
                    slowdownPowerups.push_back({Rectangle{static_cast<float>(screenWidth) + 40.0f,
                                                          LaneToY(lane, roadTop, laneHeight, orbSize), orbSize, orbSize}});
                }
            }

            for (size_t i = 0; i < obstacleRects.size(); i++) {
                obstacleRects[i].x -= velocity * delta_time;
            }
            for (size_t i = 0; i < powerups.size(); i++) {
                powerups[i].x -= velocity * delta_time * 0.9f;
            }
            for (size_t i = 0; i < slowdownPowerups.size(); i++) {
                slowdownPowerups[i].x -= velocity * delta_time * 0.9f;
            }

            // Removing entities when out of screen
            // Backward iteration because we are erasing a vector that we are currently looping
            for (int i = static_cast<int>(obstacleRects.size()) - 1; i >= 0; i--) {
                if (obstacleRects[i].x + obstacleRects[i].width < 0.0f) {
                    obstacleRects.erase(obstacleRects.begin() + i);
                    obstacleColors.erase(obstacleColors.begin() + i);
                }
            }

            for (int i = static_cast<int>(powerups.size()) - 1; i >= 0; i--) {
                if (powerups[i].x + powerups[i].width < 0.0f) {
                    powerups.erase(powerups.begin() + i);
                }
            }

            for (int i = static_cast<int>(slowdownPowerups.size()) - 1; i >= 0; i--) {
                if (slowdownPowerups[i].x + slowdownPowerups[i].width < 0.0f) {
                    slowdownPowerups.erase(slowdownPowerups.begin() + i);
                }
            }

            if (invulnerableTimer > 0.0f) invulnerableTimer -= delta_time;

            // Collision detection mechanism
            for (size_t i = 0; i < obstacleRects.size(); i++) {
                if (CheckCollisionRecs(playerRect, obstacleRects[i]) && invulnerableTimer <= 0.0f) {
                    // died = true;
                    break;
                }
            }

            for (size_t i = 0; i < powerups.size();) {
                if (CheckCollisionRecs(playerRect, powerups[i])) {
                    PlaySound(shieldSound);
                    invulnerableTimer = playerInvulnerableDuration;
                    score += 3000.0f;
                    powerups.erase(powerups.begin() + static_cast<int>(i));
                } else {
                    i++;
                }
            }

            for (size_t i = 0; i < slowdownPowerups.size();) {
                if (CheckCollisionRecs(playerRect, slowdownPowerups[i])) {
                    PlaySound(shieldSound);
                    velocity -= slowdownAmount;
                    if (velocity < minVelocity) velocity = minVelocity;
                    score += 1500.0f;
                    slowdownPowerups.erase(slowdownPowerups.begin() + static_cast<int>(i));
                } else {
                    i++;
                }
            }
        } else {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                StopSound(deathSound);
                died = false;
                deathSoundPlayed = false;
                score = 0.0f;
                velocity = 370.0f;
                player_row = 0;
                obstacleSpawnTimer = 0.0f;
                powerupSpawnTimer = 0.0f;
                slowdownSpawnTimer = 0.0f;
                invulnerableTimer = 0.0f;
                laneOffset = 0.0f;
                obstacleRects.clear();
                obstacleColors.clear();
                powerups.clear();
                slowdownPowerups.clear();
            }
        }

        BeginDrawing();
        ClearBackground(Color{16, 190, 105, 255});

        // Draw roads
        DrawRectangle(0, static_cast<int>(roadTop), screenWidth, static_cast<int>(roadHeight), Color{124, 124, 124, 255});

        // Draw trees
        for (int x = -fmodf(laneOffset, 180.0f); x <= static_cast<float>(screenWidth) + 180.0f; x += 180.0f) {
            DrawCircle(static_cast<int>(x + 80.0f), static_cast<int>(grassBandHeight * 0.5f), 34.0f, Color{63, 128, 49, 255});
            DrawCircle(static_cast<int>(x + 80.0f), static_cast<int>(screenHeight - grassBandHeight * 0.5f), 34.0f, Color{63, 128, 49, 255});
        }

        // Draw lane dash
        for (int split = 1; split <= 2; split++) {
            const int y = static_cast<int>(roadTop + laneHeight * split);
            for (float x = -fmodf(laneOffset, 120.0f); x < static_cast<float>(screenWidth) + 120.0f; x += 120.0f) {
                DrawRectangle(static_cast<int>(x), y - 6, 78, 12, RAYWHITE);
            }
        }

        // Draw player
        const Color ringColor = (invulnerableTimer > 0.0f) ? Color{80, 235, 255, 255} : Color{133, 77, 255, 255};
        DrawCircleLines(static_cast<int>(playerRect.x + playerRect.width * 0.5f), static_cast<int>(playerRect.y + playerRect.height * 0.5f), playerRect.width * 0.5f, ringColor);
        DrawCircle(static_cast<int>(playerRect.x + playerRect.width * 0.5f), static_cast<int>(playerRect.y + playerRect.height * 0.5f), playerRect.width * 0.3f, Color{86, 86, 86, 255});

        for (size_t i = 0; i < obstacleRects.size(); i++) {
            Rectangle carRect = obstacleRects[i];
            const float wheelR = carRect.height * 0.18f;

            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.20f), static_cast<int>(carRect.y - wheelR * 0.2f), wheelR, BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.80f), static_cast<int>(carRect.y - wheelR * 0.2f), wheelR, BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.20f), static_cast<int>(carRect.y + carRect.height + wheelR * 0.2f), wheelR, BLACK);
            DrawCircle(static_cast<int>(carRect.x + carRect.width * 0.80f), static_cast<int>(carRect.y + carRect.height + wheelR * 0.2f), wheelR, BLACK);

            DrawRectangleRounded(carRect, 0.10f, 4, obstacleColors[i]);
        }

        // Pointer statement for green shield powerups
        for (const Rectangle* p = powerups.data(); p != powerups.data() + powerups.size(); p++) {
            DrawCircle(static_cast<int>(p->x + p->width / 2.0f), static_cast<int>(p->y + p->height / 2.0f), p->width * 0.5f, Color{86, 207, 94, 255});
            DrawText("+", static_cast<int>(p->x + p->width * 0.28f), static_cast<int>(p->y - p->height * 0.08f), static_cast<int>(p->height * 0.92f), DARKGREEN);
        }

        for (const Rectangle* p = slowdownPowerups.data(); p != slowdownPowerups.data() + slowdownPowerups.size(); p++) {
            DrawCircle(static_cast<int>(p->x + p->width / 2.0f), static_cast<int>(p->y + p->height / 2.0f), p->width * 0.5f, Color{66, 135, 245, 255});
            DrawText("+", static_cast<int>(p->x + p->width * 0.28f), static_cast<int>(p->y - p->height * 0.08f), static_cast<int>(p->height * 0.92f), DARKBLUE);
        }

        // Render UI components
        DrawText("LATE TO CLASS!!", 24, 18, 32, RAYWHITE);
        DrawText(TextFormat("Score: %i", static_cast<int>(score)), 24, 58, 26, Color{255, 241, 170, 255});
        DrawText(TextFormat("Speed: %.2f m/s", velocity), 24, 88, 22, RAYWHITE);
        DrawText("Move: UP/DOWN or W/S", 24, 114, 20, RAYWHITE);

        if (invulnerableTimer > 0.0f) {
            DrawText(TextFormat("Shield: %.1fs", invulnerableTimer), 24, 164, 22, Color{104, 237, 255, 255});
        }

        if (died) {
            if (!deathSoundPlayed) {
                PlaySound(deathSound);
                deathSoundPlayed = true;  // Set to true so it doesn't play next frame
            }
            DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 140});
            DrawText("YOU DIED", screenWidth / 2 - 116, screenHeight / 2 - 74, 50, RED);
            DrawText(TextFormat("Final Score: %i", static_cast<int>(score)), screenWidth / 2 - 114, screenHeight / 2 - 16, 30, RAYWHITE);
            DrawText("Press R / ENTER / Left Click to Restart", screenWidth / 2 - 206, screenHeight / 2 + 36, 24, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadMusicStream(bgMusic);
    UnloadSound(deathSound);
    UnloadSound(shieldSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
