// Bomb.h
// Purpose: Defines the Bomb class — a projectile launched by the Bomber
// (Enemy Type 3) toward a locked target position.
//
// Unlike EnemyProjectile (which fires along a direction), a Bomb travels
// toward a fixed TARGET POINT and stops when it arrives. Once launched it
// is fully independent of the Bomber — Game owns it in a collection, so
// destroying the Bomber never affects an in-flight bomb.
//
// Attributes:
// - m_position: current world-space position.
// - m_target: locked destination (chosen at launch, never changes).
// - m_direction: unit vector toward the target, computed once at launch.
// - m_speed: travel speed in px/s.
// - m_lifetimeRemaining: safety expiry so a bomb can never live forever.
// - m_inWarning: true after reaching the target — warning phase (Phase 5).
// - m_warningTimer: counts down BOMB_WARNING_DURATION, then explodes.
// - m_exploding: true during the explosion animation (Phase 6).
// - m_explosionTimer: counts down BOMB_EXPLOSION_DURATION, then finishes.
// - m_detonated: one-shot flag set at detonation; Game consumes it to
//   apply blast damage exactly once.
// - m_finished: true once the explosion animation completes — Game sweep.
//
// Lifecycle: Flying -> Warning -> Explosion -> Finished.
//
// Methods:
// - Update(dt): move / warn / explode / finish, all delta-time driven.
// - Draw(): glowing orb in flight, red warning circle, then explosion.
// - IsFinished(): removal query for Game's sweep.
// - ConsumeDetonation(): returns true exactly once, at the detonation frame.
// - IsInsideBlast(pos): whether pos is within the explosion radius.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Bomb Tuning (Phases 4-6 — launch, warning, explosion + blast damage)
// ----------------------------------------------------------------------------
constexpr float BOMB_SPEED = 300.0f;      // px/s — visible, dodgeable travel
constexpr float BOMB_RADIUS = 8.0f;       // orb radius
constexpr float BOMB_LIFETIME = 8.0f;     // seconds — safety expiry
constexpr float BOMB_WARNING_DURATION = 1.5f; // seconds the warning circle shows
constexpr float BOMB_EXPLOSION_RADIUS = 90.0f;// warning circle radius = blast area
constexpr float BOMB_EXPLOSION_DURATION = 0.5f;// seconds of explosion animation

class Bomb
{
private:
    Vector2 m_position;
    Vector2 m_target;
    Vector2 m_direction; // locked unit vector — trajectory never changes
    float   m_speed;
    float   m_lifetimeRemaining;
    float   m_warningTimer;
    bool    m_inWarning;
    bool    m_exploding;
    float   m_explosionTimer;
    bool    m_detonated;
    bool    m_finished;

public:
    Bomb(Vector2 spawnPos, Vector2 targetPos, float speed = BOMB_SPEED);

    void Update(float dt);
    void Draw() const;

    bool IsFinished() const;

    // Returns true exactly once — on the frame the bomb detonates.
    // Game uses this to apply blast damage a single time.
    bool ConsumeDetonation();

    // True if worldPos lies inside (or on) the explosion radius.
    bool IsInsideBlast(Vector2 worldPos) const;
};
