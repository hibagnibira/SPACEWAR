// EnemyProjectile.h
// Purpose: Projectile fired by enemy spaceships at the player.
//
// Fixed-trajectory shot (no homing) with a lightweight fading trail:
// a fixed-size ring buffer of past positions emitted every TRAIL_SPACING
// pixels of travel — no per-frame allocations, no particle system.
//
// Attributes:
// - position / velocity: world-space motion (px/s).
// - spawnPosition / distanceTraveled: travel bookkeeping for expiry.
// - lifetimeRemaining: seconds before the shot expires.
// - m_trail / m_trailHead / m_trailCount: ring buffer of trail points.
// - m_color: per-type tint so Type 1 and Type 2 shots are distinguishable.
//
// Methods:
// - Update(dt, worldW, worldH): move, age trail, expire on limits/world edge.
// - Draw(): fading trail + head circle (world space).
// - Touches(): circle test vs the player ship.
// - IsExpired() / Destroy(): lifetime management for Game's sweep.

#pragma once

#include "raylib.h"
#include <array>

class Player; // forward declaration — damage applied via behavior method

// ----------------------------------------------------------------------------
// Enemy Projectile & Trail Tuning
// ----------------------------------------------------------------------------
constexpr int   ENEMY_PROJ_TRAIL_POINTS  = 24;     // ring buffer capacity
constexpr float ENEMY_PROJ_TRAIL_SPACING = 8.0f;   // px of travel per trail point
constexpr float ENEMY_PROJ_TRAIL_LIFE    = 0.3f;   // seconds a trail point persists
constexpr float ENEMY_PROJ_RADIUS        = 4.5f;   // projectile radius (compact energy bullet)
constexpr float ENEMY_PROJ_LIFETIME      = 2.5f;   // seconds before expiry
constexpr float ENEMY_PROJ_MAX_DISTANCE  = 1600.0f;// px of travel before expiry

class EnemyProjectile
{
private:
    struct TrailPoint
    {
        Vector2 pos;
        float   life;
    };

    Vector2 position;
    Vector2 velocity;
    Vector2 spawnPosition;
    float   lifetimeRemaining;
    float   distanceTraveled;
    float   distanceSinceTrail;
    std::array<TrailPoint, ENEMY_PROJ_TRAIL_POINTS> m_trail;
    int     m_trailHead;  // next ring slot to write
    int     m_trailCount; // valid points currently stored (<= capacity)
    Color   m_color;
    float   m_trailLife;  // per-instance trail duration (type identity)
    float   m_damage;     // health damage this shot deals on a player hit

public:
    // trailLifeScale multiplies the base trail life so each enemy type can
    // have a subtly different trail length (Type 2 needs more at high speed).
    EnemyProjectile(Vector2 spawnPos, Vector2 projVelocity, Color color,
                     float damage, float trailLifeScale = 1.0f);

    void Update(float dt, float worldWidth, float worldHeight);
    void Draw() const;

    bool IsExpired() const;
    void Destroy();

    // Circle overlap test vs the player (no position getter needed)
    bool Touches(Vector2 otherPos, float otherRadius) const;

    // Applies this shot's damage through the player's damage path
    // (respects invulnerability/death rules inside Player::TakeDamage)
    void ApplyToPlayer(Player& player) const;
};
