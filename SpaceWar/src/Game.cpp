// Game.cpp
// Purpose: Implements the methods declared in Game.h.
//
// This file coordinates the main game loop, state transitions, entity updates,
// rendering, collision detection, and enemy spawning.

#include "Game.h"
#include <cmath>
#include <algorithm>

// ----------------------------------------------------------------------------
// TEMPORARY DEBUG DAMAGE — remove once enemy/asteroid/boss damage exists.
// Press F1 once to deal DEBUG_DAMAGE_AMOUNT percent damage to the player.
// ----------------------------------------------------------------------------
constexpr int   DEBUG_DAMAGE_KEY = KEY_F1;
constexpr float DEBUG_DAMAGE_AMOUNT = 10.0f;

Game::Game(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_worldWidth(2400.0f)
    , m_worldHeight(1800.0f)
    , m_player({ 1200.0f, 900.0f }, 2400.0f, 1800.0f) // Spawn at center of arena
    , m_camera{ 0 }
{
    // Deterministic starfield generation (stable across runs)
    constexpr int STAR_COUNT = 400;
    m_stars.reserve(STAR_COUNT);

    unsigned int seed = 1337u;
    for (int i = 0; i < STAR_COUNT; ++i)
    {
        seed = seed * 1664525u + 1013904223u;
        const float fx = static_cast<float>(seed >> 8) / 16777216.0f;

        seed = seed * 1664525u + 1013904223u;
        const float fy = static_cast<float>(seed >> 8) / 16777216.0f;

        seed = seed * 1664525u + 1013904223u;
        const float fr = static_cast<float>(seed >> 8) / 16777216.0f;

        Star star;
        star.position = { fx * m_worldWidth, fy * m_worldHeight };
        star.radius = 0.5f + fr * 2.0f;
        star.alpha = static_cast<unsigned char>(80 + (seed & 0x7Fu));
        m_stars.push_back(star);
    }
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

    // 2. Spawn a bullet if the player requested one this frame
    if (m_player.m_shotRequested)
    {
        m_player.m_shotRequested = false;
        SpawnPlayerBullet();
    }

    // 3. Update projectiles
    UpdateBullets(dt);

    // Temporary debug damage (single press per hit, not per frame)
    if (IsKeyPressed(DEBUG_DAMAGE_KEY))
    {
        m_player.TakeDamage(DEBUG_DAMAGE_AMOUNT);
    }

    // 4. Smoothly follow player's new position
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

void Game::SpawnPlayerBullet()
{
    // Facing direction from the ship's rotation convention:
    // 0 radians = facing right, nose at +x, rear at -x.
    const Vector2 facing = { std::cos(m_player.m_rotation), std::sin(m_player.m_rotation) };

    // Spawn at the rear of the ship, offset behind center along -facing.
    constexpr float REAR_OFFSET = 20.0f;
    const Vector2 spawnPos = { m_player.m_position.x - facing.x * REAR_OFFSET,
                               m_player.m_position.y - facing.y * REAR_OFFSET };

    // Shoot backwards (180 degrees from facing) at fixed bullet speed,
    // plus the player's current velocity captured at this exact moment
    // (includes dash velocity while boosting).
    const Vector2 shootDir = { -facing.x, -facing.y };
    const Vector2 bulletVelocity = { shootDir.x * BULLET_SPEED + m_player.m_velocity.x,
                                     shootDir.y * BULLET_SPEED + m_player.m_velocity.y };

    m_bullets.emplace_back(spawnPos, bulletVelocity);
}

void Game::UpdateBullets(float dt)
{
    for (Bullet& bullet : m_bullets)
    {
        bullet.Update(dt);
    }

    // Remove expired bullets without invalidating iteration
    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
        [](const Bullet& bullet) { return bullet.IsExpired(); }),
        m_bullets.end());
}

void Game::Draw()
{
    BeginDrawing();

    // Blue space gradient backdrop (screen space, covers entire window)
    DrawRectangleGradientV(0, 0, m_screenWidth, m_screenHeight,
        Color{ 10, 25, 70, 255 },   // deep space blue at top
        Color{ 25, 70, 140, 255 }); // brighter horizon blue at bottom

    // --------------------------------------------------------------------
    // 1. World Space (Inside Camera2D Transformation)
    // --------------------------------------------------------------------
    BeginMode2D(m_camera);

    // Starfield
    for (const Star& star : m_stars)
    {
        DrawCircleV(star.position, star.radius, Color{ 190, 215, 255, star.alpha });
    }

    // World background grid
    constexpr int gridSize = 80;
    const Color gridColor = Color{ 45, 90, 150, 255 };

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

    // Player projectiles
    for (const Bullet& bullet : m_bullets)
    {
        bullet.Draw();
    }

    EndMode2D();

    // --------------------------------------------------------------------
    // 2. Screen Space (HUD / UI Outside Camera Mode)
    // --------------------------------------------------------------------
    DrawText("SPACEWAR", 20, 20, 20, RAYWHITE);
    DrawFPS(m_screenWidth - 100, 20);

    // HUD: player health bar (fixed screen position, drawn by Player)
    m_player.DrawHealthBar();

    // HUD: controls guide (update this list when bindings change)
    DrawText("CONTROLS", 20, 260, 18, RAYWHITE);
    DrawText("Mouse      - Move & aim",        20, 286, 14, GRAY);
    DrawText("Left Click - Toggle shooting",   20, 304, 14, GRAY);
    DrawText("Space      - Dash / Boost",      20, 322, 14, GRAY);
    DrawText("F1         - Debug damage (temporary)", 20, 340, 14, GRAY);

    EndDrawing();
}