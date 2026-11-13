// Enemy.h
// Purpose: Defines the Enemy class — a hostile spaceship.
//
// Milestone 2: random wandering until the enemy enters the camera's
// visible area, then permanent chasing of the player. Movement only —
// shooting, health, and collision come in later milestones.
//
// Attributes:
// - position: World-space (x, y) of the enemy.
// - radius: Collision/draw radius of the placeholder shape.
// - m_destination: current random wander target.
// - m_isChasing: latched true once the enemy has entered the camera view.
//
// Methods:
// - Update(...): wander/chase movement, detection, boundary steering.
// - Draw(): Renders the enemy placeholder in world space.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Enemy Movement Tuning (Milestone 2)
// ----------------------------------------------------------------------------
constexpr float ENEMY_SPEED_NORMAL = 240.0f;        // px/s — normal movement
constexpr float ENEMY_SPEED_PLAYER_ACCEL = 500.0f;  // px/s — while player accelerates
constexpr float ENEMY_ARRIVE_DISTANCE = 24.0f;  // px — pick a new wander target
constexpr float ENEMY_BOUNDARY_MARGIN = 60.0f;  // px — inward-steer zone at edges
constexpr float ENEMY_BOUNDARY_STEER = 1.5f;    // inward bias strength near edges

class Enemy
{
private:
    Vector2 position;
    float   radius;
    Vector2 m_destination;   // current random wander target
    bool    m_isChasing;     // true once the enemy has entered the camera view

    // Picks a new random destination inside the given bounds
    void PickNewDestination(float minX, float minY, float maxX, float maxY);

public:
    Enemy(Vector2 spawnPos, float size = 20.0f);

    void Update(float dt, Vector2 playerPos, const Camera2D& camera,
                float worldWidth, float worldHeight,
                bool playerAccelerating, bool playerDead);
    void Draw() const;
};
