// Game.cpp
// Purpose: Implements the methods declared in Game.h.
//
// This file coordinates the main game loop, state transitions, entity updates,
// rendering, collision detection, and enemy spawning.

#include "Game.h"
#include <cmath>

Game::Game(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_worldWidth(2400.0f)
    , m_worldHeight(1800.0f)
    , m_player({ 1200.0f, 900.0f }, 2400.0f, 1800.0f) // Spawn at center of arena
    , m_camera{ 0 }
{
}

void Game::Init()
{
    // ------------------------------------------------------------------------
    // Camera2D Initialization
    // Directly targets m_player.m_position via friendship (zero getters used)
    // ------------------------------------------------------------------------
    m_camera.target = m_player.m_position;
    m_camera.offset = Vector2{ static_cast<float>(m_screenWidth) / 2.0f,
                                 static_cast<float>(m_screenHeight) / 2.0f }; // Center on screen
    m_camera.rotation = 0.0f; // Fixed 0 rotation
    m_camera.zoom = 1.5f; // Fixed 1.5x zoom
}

void Game::Update(float dt)
{
    if (dt <= 0.0f) return;

    // 1. Update player (player reads camera to calculate world-space mouse target)
    m_player.Update(dt, m_camera);

    // 2. Smoothly follow player's new position
    UpdateCamera(dt);
}

void Game::UpdateCamera(float dt)
{
    constexpr float CAMERA_FOLLOW_SPEED = 5.0f;
    const float cameraBlend = 1.0f - std::exp(-CAMERA_FOLLOW_SPEED * dt);

    // Read m_player.m_position directly through class friendship (no getters)
    m_camera.target.x += (m_player.m_position.x - m_camera.target.x) * cameraBlend;
    m_camera.target.y += (m_player.m_position.y - m_camera.target.y) * cameraBlend;
}

void Game::Draw()
{
    BeginDrawing();
    ClearBackground(Color{ 8, 10, 18, 255 });

    // --------------------------------------------------------------------
    // 1. World Space (Inside Camera2D Transformation)
    // --------------------------------------------------------------------
    BeginMode2D(m_camera);

    // World background grid
    constexpr int gridSize = 80;
    const Color gridColor = Color{ 25, 30, 45, 255 };

    for (int x = 0; x <= static_cast<int>(m_worldWidth); x += gridSize)
    {
        DrawLine(x, 0, x, static_cast<int>(m_worldHeight), gridColor);
    }
    for (int y = 0; y <= static_cast<int>(m_worldHeight); y += gridSize)
    {
        DrawLine(0, y, static_cast<int>(m_worldWidth), y, gridColor);
    }

    // World boundary border (red outline)
    DrawRectangleLinesEx(Rectangle{ 0.0f, 0.0f, m_worldWidth, m_worldHeight }, 4.0f, Fade(RED, 0.6f));

    // Mouse target reticle in world space
    const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), m_camera);
    DrawCircleLinesV(mouseWorldPos, 10.0f, Color{ 80, 160, 240, 140 });
    DrawCircleV(mouseWorldPos, 2.0f, Color{ 120, 200, 255, 200 });

    // Spaceship
    m_player.Draw();

    EndMode2D();

    // --------------------------------------------------------------------
    // 2. Screen Space (HUD / UI Outside Camera Mode)
    // --------------------------------------------------------------------
    DrawText("SPACEWAR", 20, 20, 20, RAYWHITE);
    DrawFPS(m_screenWidth - 100, 20);

    EndDrawing();
}