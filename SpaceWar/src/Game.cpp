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

// ----------------------------------------------------------------------------
// Healing Stone Spawning (initial balancing values)
// ----------------------------------------------------------------------------
// No stones spawn while player health is above 50% — the timer re-checks
// periodically so spawning resumes as soon as health drops to 50% or below.
constexpr float HEALING_STONE_SPAWN_RECHECK_INTERVAL = 1.0f;
constexpr float HEALING_STONE_SPAWN_INTERVAL_MID  = 5.0f;  // health 31%-50%
constexpr float HEALING_STONE_SPAWN_INTERVAL_LOW  = 3.0f;  // health <= 30%
constexpr float HEALING_STONE_MIN_PLAYER_DISTANCE = 100.0f; // world units
constexpr int   HEALING_STONE_MAX_SPAWN_ATTEMPTS  = 20;

Game::Game(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_worldWidth(2400.0f)
    , m_worldHeight(1800.0f)
    , m_player({ 1200.0f, 900.0f }, 2400.0f, 1800.0f) // Spawn at center of arena
    , m_camera{ 0 }
    , m_stoneSpawnTimer(0.0f)
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

    // 3b. Update healing stones (spawn timer, lifetime, collection)
    UpdateHealingStones(dt);

    // Temporary debug damage (single press per hit, not per frame)
    if (IsKeyPressed(DEBUG_DAMAGE_KEY))
    {
        m_player.TakeDamage(DEBUG_DAMAGE_AMOUNT);
    }

    // Temporary respawn: R only works while the player is dead (single press)
    if (m_player.m_isDead && IsKeyPressed(KEY_R))
    {
        m_player.Respawn(m_camera);
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

    // Back mode: spawn at rear, fire opposite facing.
    // Front mode: spawn at nose, fire along facing.
    const bool frontMode = m_player.m_frontShooting;
    const float spawnOffset = frontMode ? 24.0f : 20.0f; // nose vs rear offset
    const float dirSign = frontMode ? 1.0f : -1.0f;

    const Vector2 spawnPos = { m_player.m_position.x + facing.x * dirSign * spawnOffset,
                               m_player.m_position.y + facing.y * dirSign * spawnOffset };

    // Shooting velocity + player's current velocity captured at this moment
    // (includes acceleration velocity while boosting).
    const Vector2 shootDir = { facing.x * dirSign, facing.y * dirSign };
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

// ----------------------------------------------------------------------------
// Healing Stones
// ----------------------------------------------------------------------------

void Game::TrySpawnHealingStone()
{
    // Spawn rules scale with current player health (Game is a Player friend)
    const float health = m_player.m_health;

    if (health > 50.0f)
    {
        // Above 50%: no spawning. Re-check soon so spawning resumes
        // promptly when health drops to 50% or below.
        m_stoneSpawnTimer = HEALING_STONE_SPAWN_RECHECK_INTERVAL;
        return;
    }

    float interval = HEALING_STONE_SPAWN_INTERVAL_MID; // 31%-50%
    if (health <= 30.0f)
    {
        interval = HEALING_STONE_SPAWN_INTERVAL_LOW;
    }
    m_stoneSpawnTimer = interval;

    Vector2 spawnPos;
    if (TryFindStoneSpawnPos(spawnPos))
    {
        m_healingStones.emplace_back(spawnPos);
    }
    // If no valid spot was found, this spawn attempt is safely skipped
}

bool Game::TryFindStoneSpawnPos(Vector2& outPos) const
{
    // Visible world rectangle: camera.target is the smoothed view center,
    // zoom scales the visible half-extents around it.
    const float halfViewW = (m_screenWidth * 0.5f) / m_camera.zoom;
    const float halfViewH = (m_screenHeight * 0.5f) / m_camera.zoom;

    // Intersect the view rect with the playable world bounds
    const float minX = std::max(m_camera.target.x - halfViewW, 0.0f);
    const float maxX = std::min(m_camera.target.x + halfViewW, m_worldWidth);
    const float minY = std::max(m_camera.target.y - halfViewH, 0.0f);
    const float maxY = std::min(m_camera.target.y + halfViewH, m_worldHeight);

    if (maxX <= minX || maxY <= minY)
    {
        return false;
    }

    const float minDistSq = HEALING_STONE_MIN_PLAYER_DISTANCE * HEALING_STONE_MIN_PLAYER_DISTANCE;

    // Limited attempts: give up rather than risk an infinite loop
    for (int attempt = 0; attempt < HEALING_STONE_MAX_SPAWN_ATTEMPTS; ++attempt)
    {
        const float rx = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;
        const float ry = static_cast<float>(GetRandomValue(0, 1000000)) / 1000000.0f;

        const float x = minX + rx * (maxX - minX);
        const float y = minY + ry * (maxY - minY);

        const float dx = x - m_player.m_position.x;
        const float dy = y - m_player.m_position.y;
        if (dx * dx + dy * dy >= minDistSq)
        {
            outPos = { x, y };
            return true;
        }
    }

    return false;
}

void Game::UpdateHealingStones(float dt)
{
    // While the player is dead the whole system freezes: no spawning,
    // no lifetime countdown, no collection. Stones keep their positions
    // and resume normally after respawn.
    if (m_player.m_isDead)
    {
        return;
    }

    // Spawn countdown; interval is re-evaluated from health at each spawn
    m_stoneSpawnTimer -= dt;
    if (m_stoneSpawnTimer <= 0.0f)
    {
        TrySpawnHealingStone();
    }

    // Age stones, handle collection, and remove expired/collected safely
    for (auto it = m_healingStones.begin(); it != m_healingStones.end(); )
    {
        it->Update(dt);

        if (it->IsExpired())
        {
            it = m_healingStones.erase(it);
            continue;
        }

        // Collection: stone radius + ship collision radius overlap
        if (it->Overlaps(m_player.m_position, SHIP_COLLISION_RADIUS))
        {
            m_player.Heal(HEALING_AMOUNT);
            it = m_healingStones.erase(it);
            continue;
        }

        ++it;
    }
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

    // Healing stones
    for (const HealingStone& stone : m_healingStones)
    {
        stone.Draw();
    }

    EndMode2D();

    // --------------------------------------------------------------------
    // 2. Screen Space (HUD / UI Outside Camera Mode)
    // --------------------------------------------------------------------
    DrawText("SPACEWAR", 20, 20, 20, RAYWHITE);
    DrawFPS(m_screenWidth - 100, 20);

    // HUD: player health bar (fixed screen position, drawn by Player)
    m_player.DrawHealthBar();
    m_player.DrawEnergyBar();

    // HUD: current shooting mode (screen space, updates instantly on S)
    DrawText(TextFormat("SHOOTING MODE: %s", m_player.m_frontShooting ? "FRONT" : "BACK"),
             20, 116, 16, m_player.m_frontShooting ? YELLOW : LIGHTGRAY);

    // HUD: controls guide (update this list when bindings change)
    DrawText("CONTROLS", 20, 260, 18, RAYWHITE);
    DrawText("Mouse             - Move & aim",               20, 286, 14, GRAY);
    DrawText("Left Click        - Toggle Shooting",          20, 304, 14, GRAY);
    DrawText("Left Double-Click - Activate Acceleration",    20, 322, 14, GRAY);
    DrawText("Space             - Accelerate (energy)",      20, 340, 14, GRAY);
    DrawText("S                 - Switch Shooting Mode",     20, 358, 14, GRAY);
    DrawText("F1                - Debug damage (temporary)",20, 376, 14, GRAY);
    if (m_player.m_isDead)
    {
        DrawText("R                - Respawn (temporary)",   20, 394, 14, YELLOW);
    }

    // GAME OVER overlay — visible only while the player is dead
    if (m_player.m_isDead)
    {
        constexpr const char* gameOverText = "GAME OVER";
        constexpr int fontSize = 64;
        const int textWidth = MeasureText(gameOverText, fontSize);
        DrawText(gameOverText,
                 (m_screenWidth - textWidth) / 2,
                 m_screenHeight / 2 - fontSize / 2,
                 fontSize, RED);
    }

    EndDrawing();
}