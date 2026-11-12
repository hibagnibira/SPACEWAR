// Bullet.h
// Purpose: Defines the Bullet class representing projectiles fired by the
// player or by enemy spaceships.
//
// Attributes:
// - position: Current (x, y) position of the projectile.
// - velocity: Full velocity vector (shooting velocity + inherited player
//   movement velocity captured at fire time).
// - spawnPosition: Position where the bullet was created, used to measure
//   total travel distance.
// - lifetimeRemaining: Seconds left before the bullet expires.
// - distanceTraveled: Total distance covered so far in pixels.
//
// Methods:
// - Update(float deltaTime): Moves the projectile and ages its timers.
// - Draw(): Renders a simple placeholder circle.
// - IsExpired(): True when lifetime OR max travel distance is reached.
// - GetBounds(): Collision rectangle for future enemy hit detection.

#pragma once

#include "raylib.h"

// ----------------------------------------------------------------------------
// Projectile Tuning Constants
// ----------------------------------------------------------------------------
constexpr float BULLET_SPEED = 900.0f;         // px/s — fast shot speed; tune after testing
constexpr float BULLET_LIFETIME = 3.0f;        // seconds before a missed bullet expires
constexpr float BULLET_MAX_DISTANCE = 3000.0f; // px of travel before a bullet expires
constexpr float BULLET_RADIUS = 4.0f;          // placeholder projectile radius

class Bullet
{
private:
    Vector2 position;
    Vector2 velocity;
    Vector2 spawnPosition;
    float lifetimeRemaining;
    float distanceTraveled;

public:
    Bullet(Vector2 spawnPos, Vector2 bulletVelocity);

    void Update(float deltaTime);
    void Draw() const;

    bool IsExpired() const;
    Rectangle GetBounds() const;
};
